# PS/PL TR-SPI FIFO 技术说明

文档状态：与 2026-08-27 当前代码一致。

## 1. 范围与源码入口

本文说明 PS 端软件、AXI 寄存器、四个 FIFO 以及 TX/RX SPI 状态机之间的实际数据流。

手工维护时应以以下目录为准：

- PS 软件：`atcl/trctrl.c`、`atcl/trctrl.h`、`atcl/beam.c`
- Ku_TR_SPI HDL/IP：工程根目录 `Ku_IP/ip_repo/Ku_TR_SPI_1.0`
- AXI 接口：`Ku_IP/ip_repo/Ku_TR_SPI_1.0/hdl/Ku_TR_SPI_v1_0_S00_AXI.v`
- TX/RX 队列状态机：`Ku_IP/ip_repo/Ku_TR_SPI_1.0/src/tx.v`、`src/rx.v`
- SPI 与 LDAC 时序：`src/tx_spi_inf.v`、`src/rx_spi_inf.v`

`GT2001_0316.srcs` 下的同名 HDL 是 Vivado 生成或复制的工程产物，不是源码维护入口。

## 2. 数据通路

系统有四个 24-bit、深度 2048 的 FIFO：

| 方向 | 命令 FIFO（PS -> PL） | 返回 FIFO（PL -> PS） |
| --- | --- | --- |
| TX | `fifo_TX_t` | `fifo_TX_r` |
| RX | `fifo_RX_t` | `fifo_RX_r` |

正常链路如下：

1. PS 通过 AXI 把 24-bit 命令写入 TX 或 RX 命令 FIFO。
2. `tx.v` / `rx.v` 在 SPI 空闲且命令 FIFO 非空时发起一次出队。
3. 默认状态机等待 FIFO 的 `valid`，锁存数据后启动 SPI，并等待 `busy` 拉高再拉低。
4. SPI 回读有效时，结果写入对应返回 FIFO。
5. PS 读取返回 FIFO 计数，再从对应数据寄存器取结果。

TX/RX 的幅相写在各自 `*_spi_inf.v` 的单帧结束状态处理 LDAC，不依赖“整批 FIFO 发送完毕”才触发。

## 3. AXI 寄存器映射

基地址由 `XPAR_KU_TR_SPI_0_S00_AXI_BASEADDR` 提供，当前为 `0x43C20000`。

| 偏移 | 写入语义 | 读取语义 |
| --- | --- | --- |
| `REG0 0x00` | TX/RX 复位控制 | 0 |
| `REG1 0x04` | 写 TX 命令 FIFO | 读 TX 返回数据并出队 |
| `REG2 0x08` | 普通寄存器写入无业务用途 | TX 返回 FIFO 可读计数 |
| `REG3 0x0C` | 普通寄存器写入无业务用途 | TX 命令 FIFO 已用计数 |
| `REG4 0x10` | 写 RX 命令 FIFO | 读 RX 返回数据并出队 |
| `REG5 0x14` | 普通寄存器写入无业务用途 | RX 返回 FIFO 可读计数 |
| `REG6 0x18` | 普通寄存器写入无业务用途 | RX 命令 FIFO 已用计数 |
| `REG7 0x1C` | 保留控制寄存器 | 原值读回 |

PS 侧常用对应关系：

- `TX_rx_fifo_valid()`：读 `REG2`
- `TX_tx_fifo_valid()`：根据 `REG3` 计算保守可写空间
- `RX_rx_fifo_valid()`：读 `REG5`
- `RX_tx_fifo_valid()`：根据 `REG6` 计算保守可写空间

## 4. 深度和计数语义

四个 XCI 的读写深度均为 2048，计数端口为 11 bit。软件使用：

```c
TR_SPI_FIFO_DATA_COUNT_MASK = 0x7FFU
TR_SPI_FIFO_COUNT_MAX       = 2047U
```

因此软件把 `0..2046` 视为仍有空间，把 `2047` 视为不可再写。这样会保守地预留一个槽位，同时避开 11-bit 计数在 2048 深度边界上的表示歧义。PL 仍使用 FIFO 自身的 `full` 信号做最终保护。

`fifo_compat.v` 也把满状态计数钳位为 2047，但该兼容实现默认被 `KU_TR_SPI_ENABLE_FIFO_COMPAT` 条件编译关闭。正常综合使用 XCI 生成的 FIFO；只有生成 FIFO 模块确实不可用时，才应显式启用兼容实现。兼容实现会用寄存器数组描述存储，不适合作为本工程默认综合方案。

## 5. PL 侧满保护与状态机

`Ku_TR_SPI_v1_0_S00_AXI.v` 对 `REG1` 和 `REG4` 的写请求检查目标命令 FIFO 的 `full`：

- TX 满时不接受 TX 写地址/数据握手。
- RX 满时不接受 RX 写地址/数据握手。
- 真正写 FIFO 时，写使能再次由 `~fifo_full_*` 门控。

这能避免满队列继续写入，但也意味着：如果软件绕过可写空间检查，而 PL 又永久停止消费，AXI 写事务可能等待。所有业务代码应继续通过 `tr_tx_write_checked()` / `tr_rx_write_checked()` 或它们的封装进入。

默认 TX/RX 命令状态机按“请求出队 -> 等待 `valid` -> 启动 SPI -> 等待 `busy` 完成”的顺序运行，避免把 FIFO 出队和 SPI 启动错误地压在同一拍。旧行为只在显式定义 `KU_TR_SPI_LEGACY_FIFO_LAUNCH` 时启用。

返回 FIFO 的写使能直接来自 SPI 回读有效脉冲，返回 FIFO 的 `full` 没有用于阻断该脉冲。因此，持续产生回读却长期不读取结果会带来返回数据溢出的风险；读命令和诊断流程应及时消费返回数据。

## 6. PS 写入接口

`tr_fifo_wait_mode_set(enable)` 决定“命令 FIFO 没空间”时是否轮询：

- `enable == 0`：只检查一次；没空间立即返回 `TR_SPI_ERR_TIMEOUT`。
- `enable != 0`：按 1 us 间隔轮询，直到调用方给出的 `timeout_us` 用尽。

默认宏为：

- `TR_SPI_POLL_INTERVAL_US = 1`
- `TR_SPI_WRITE_TIMEOUT_US = 50000`
- `TR_SPI_READ_TIMEOUT_US = 50000`

主要接口：

- `tr_tx_write_checked()` / `tr_rx_write_checked()`：返回 `TR_SPI_OK` 或 `TR_SPI_ERR_TIMEOUT`。
- `tr_tx_write()` / `tr_rx_write()`：兼容旧调用方式；函数本身无返回值，失败时只打印超时信息。
- `tr_tx_read_checked()` / `tr_rx_read_checked()`：先清空对应的旧返回数据，再提交命令并轮询新结果；空指针返回 `TR_SPI_ERR_PARAM`。
- `tr_tx_read()` / `tr_rx_read()`：失败时打印日志并返回初始值 0，调用方不能仅凭返回值区分“真实 0”和“超时”。需要可靠判错时应使用 `*_checked()`。

注意：`tr_spi_fifo_drain_all()` 只清空 TX/RX 的返回 FIFO，不会清空仍在 PL 命令 FIFO 中排队的命令。这个函数名中的 `all` 指两个方向的返回队列，不代表四个 FIFO 全部被清空。

## 7. 当前初始化策略

`beam_init()` 在 FreeRTOS 调度器和 UART 任务启动之前执行。当前代码明确调用：

```c
tr_fifo_wait_mode_set(0U);
```

然后才进行通道状态复位、旧返回数据清理、TX/RX 初始化和全通道命令。初始化结束再次设置为 0。因此，当前初始化不是“等待 50 ms 保证写入”的模式，而是单次检查的非阻塞模式。

这样做优先保证：即使 PL 侧故障或命令 FIFO 不再消费，系统也能继续启动到串口任务。代价是初始化突发期间如果软件看到 FIFO 无空间，该帧会失败并打印超时；使用无返回值封装的上层调用不会继续向调用者传播错误。

`trctrl.c` 中静态变量的初值是 1，但 `beam_init()` 会在任何 TR 初始化写入前把它改为 0。不要仅根据静态初值判断实际运行模式。

如果以后决定在调度器启动后的受控流程中使用等待模式，应显式成对调用 `tr_fifo_wait_mode_set(1U)` 和 `tr_fifo_wait_mode_set(0U)`，并确保 50 ms 最坏阻塞满足任务时延要求。

## 8. 故障定位

出现“命令无效、通道未打开或读回超时”时，建议按以下顺序检查：

1. 确认用 `1_BIT.bat` 生成了新 BIT，且 SDK 硬件平台只引用 `system_wrapper_hw_platform_0`。
2. 观察 `REG3` / `REG6`：命令计数是否入队并随 PL 消费而下降。
3. 观察 `REG2` / `REG5`：SPI 回读是否进入返回 FIFO。
4. 查看串口是否有 `TR-SPI TX/RX write timeout`、`read timeout` 或 `channel switch FIFO timeout`。
5. 检查 TX/RX 供电、复位、SPI `busy` 完成路径及对应引脚约束。
6. 确认没有启用 `KU_TR_SPI_LEGACY_FIFO_LAUNCH` 或误启用兼容 FIFO。
7. 读取结果前确认没有其他任务并发访问同一方向的返回 FIFO；当前“先清旧数据再等新结果”不是多任务事务仲裁机制。

## 9. 维护约束

- 修改 FIFO 深度时，必须同步 XCI、HDL 计数端口、`trctrl.h` 掩码/上限和本文档。
- 修改寄存器偏移时，必须同步 IP 驱动头文件与 `trctrl.h`。
- 新增可靠控制路径时，优先保留 `*_checked()` 返回值，不要在中间层降级成仅打印日志。
- 构建成功与否必须看 Vivado `impl_1` 状态，不能只看旧 BIT 文件是否仍存在。
