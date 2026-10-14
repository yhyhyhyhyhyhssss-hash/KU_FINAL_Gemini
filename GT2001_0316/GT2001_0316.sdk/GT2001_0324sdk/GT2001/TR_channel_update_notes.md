# TR 通道控制与初始化当前实现说明

文档状态：与 2026-08-27 当前代码一致。本文描述当前行为，不再沿用早期“初始化开启 FIFO 重试 10 次”的历史方案。

## 1. 关键入口

- 初始化：`atcl/beam.c` 中的 `beam_init()`
- 周期命令处理：`atcl/beam.c` 中的 `beam_ctr()`
- 通道掩码下发：`token_id_scc_fram()`
- 全阵面开关：`token_id_ope_fram()`
- FIFO/API：`atcl/trctrl.c`、`atcl/trctrl.h`

HDL 的唯一维护源位于工程根目录 `Ku_IP/ip_repo/Ku_TR_SPI_1.0`。不要直接修改 `GT2001_0316.srcs` 内的生成副本。

## 2. `beam_init()` 当前顺序

初始化按以下顺序执行：

1. 打印 `KU_FINAL SERIALFIX 2026-07-23`。
2. 调用 `tr_fifo_wait_mode_set(0U)`，把 TR-SPI 写入设为非阻塞单次检查。
3. 调用 `tr_channel_state_reset()`，清空 16 颗 TX 和 16 颗 RX 芯片的通道掩码缓存。
4. 调用 `tr_spi_fifo_drain_all()`，清掉两个返回 FIFO 中的旧数据。它不会清空 PL 命令 FIFO。
5. 打开公共电源和射频使能：`VCC_5V_DOWN_D_EN_H`、`VCC_55V_D_EN_H`、`VCC_5V_UP_D_EN_H`、`POWER_RF_EN_H`。
6. `RF_RX_EN == 1` 时打开 RX 相关 1 V、1.5 V 和 3.3 V 电源。
7. `RF_TX_EN == 1` 时控制 `LMZ31520RLGT_EN_L`，随后等待 10 ms。
8. TX 启用时执行 `tr_tx_init()`，等待 10 ms，打印 `KU_FINAL TX init queued`，再执行 `token_id_ope_fram(OPE_TX_ALL_ON)`。
9. RX 启用时执行 `tr_rx_init()`，等待 10 ms，打印 `KU_FINAL RX init queued`，再执行 `token_id_ope_fram(OPE_RX_ALL_ON)`。
10. 再次保持非阻塞模式，并打印 `KU_FINAL beam init done`。

被注释的 `token_chl_ctrl(...)` 和直接 `tr_tx_write(...)` 只保留为调试参考，不参与默认初始化。

## 3. 为什么初始化采用非阻塞模式

`beam_init()` 发生在 FreeRTOS 调度器和 UART 任务启动之前。如果这里等待一个永远不再消费数据的 PL FIFO，整机可能停在启动阶段，串口也无法提供诊断信息。

所以当前策略是“可诊断启动优先”：

- FIFO 有空间：立即入队。
- FIFO 没空间：当前帧失败，打印超时，继续执行启动流程。
- 初始化结束后仍保持非阻塞，除非其他代码显式开启等待模式。

这不是无损保证。若初始化突发期间 FIFO 确实无空间，使用 `tr_tx_write()` / `tr_rx_write()` 的调用只会获得日志，不会把失败继续返回给上层。需要确保某条命令成功时，应调用 `tr_tx_write_checked()` / `tr_rx_write_checked()` 并处理返回值。

等待模式仍然可用：开启后按 1 us 轮询，默认写和读超时均为 50000 us；它不是固定重试 10 次。是否开启应由任务时延和故障恢复策略决定。

## 4. 通道状态缓存

软件分别维护：

- `tx_channel_state[16]`
- `rx_channel_state[16]`

每个元素是对应芯片的 8-bit 通道掩码。缓存用于正确实现单通道开/关：

- 开通请求：`新掩码 = 旧掩码 | 请求掩码`
- 关闭请求：`新掩码 = 旧掩码 & ~请求掩码`

`token_id_scc_fram()` 先校验方向和芯片编号，再通过 checked 写接口下发；只有写入成功才更新缓存。失败时缓存保持旧值，并打印 `channel switch FIFO timeout`。这避免“硬件命令没入队，但软件已认为状态改变”的失配。

缓存表示软件成功入队的最后状态，不等于芯片真实回读状态。上电、PL 复位、外部复位或 SPI 执行失败后，如需强一致性，应重新初始化或增加实际寄存器回读校验。

## 5. 全阵面操作码

当前宏和行为如下：

| 操作码宏 | 值 | 行为 |
| --- | --- | --- |
| `OPE_RX_ALL_ON` | `0x00` | 16 颗 RX 芯片全部写 `0xFF` |
| `OPE_TX_ALL_ON` | `0xFF` | 16 颗 TX 芯片全部写 `0xFF` |
| `OPE_TX_ALL_OFF` | `0x1F` | 16 颗 TX 芯片全部写 `0x00` |
| `OPE_RX_ALL_OFF` | `0x10` | 16 颗 RX 芯片全部写 `0x00` |
| `OPE_RXTX_ALL_ON` | `0x0F` | RX、TX 全部写 `0xFF` |
| `OPE_RXTX_ALL_OFF` | `0xF0` | RX、TX 全部写 `0x00` |

未知操作码会打印 `unsupported OPE code`，不会下发命令。

## 6. 与 FIFO 的关系

一次 TX 或 RX 全开会连续下发 16 帧；TX/RX 同时全开会下发 32 帧。命令 FIFO 的物理深度是 2048，软件保守使用最大计数 2047，因此正常空队列下容量充足。

真正风险不是这 16 或 32 帧本身，而是以下情况：

- PL SPI 状态机停止消费，命令长期堆积。
- 其他任务同时大量写入同一 FIFO。
- 软件缓存和硬件在复位后未重新同步。
- 返回 FIFO 长期不读取，回读数据不断累积。

`tr_spi_fifo_drain_all()` 只清返回 FIFO。若要清命令 FIFO，应使用硬件复位路径并确认 TX/RX 状态机重新进入空闲态，不能把该函数当成四队列复位。

## 7. 调试检查表

通道未打开或电流异常时，按以下顺序检查：

1. 启动日志是否依次出现 SERIALFIX、TX/RX init queued 和 beam init done。
2. 对应方向的电源与 RF 使能是否实际到位。
3. 串口是否有 TX/RX write timeout 或 channel switch FIFO timeout。
4. AXI `REG3` / `REG6` 的命令计数是否入队并下降。
5. SPI `busy` 是否完成，LDAC 是否在目标帧结束时产生。
6. 读回路径需要时，检查 `REG2` / `REG5` 是否有结果并及时消费。
7. 复位后确认执行了通道缓存清零和 TX/RX 重新初始化。
8. 确认下载的是本次构建的 BIT 和 ELF，硬件平台只使用 `system_wrapper_hw_platform_0`。

## 8. 维护建议

- 不要用不断增加固定延时来掩盖 FIFO、SPI 或供电状态机故障。
- 对关键配置帧使用 checked 接口，并在调用链上保留错误返回。
- 如果需要初始化阶段可靠重试，建议放到调度器启动后的独立状态机中，带总超时、失败计数和可观察状态。
- 修改全阵面操作码或方向定义时，同步 `beam.h`、解析协议和本文档。
- 修改通道写逻辑时，保留“成功入队后才更新缓存”的约束。
