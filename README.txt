KU_FINAL 构建与下载说明
文档更新：2026-08-27

一、环境

- 工程使用 Xilinx Vivado / SDK 2018.3。
- 1_BIT.bat 应在 Xilinx 2018.3 Command Prompt 中运行。
- 2_ELF.bat 应在 Xilinx SDK 2018.3 Command Prompt 中运行。

二、标准构建流程

1. 运行 1_BIT.bat，重新生成 PL bitstream。
2. 运行 2_ELF.bat，clean 后重新生成应用 ELF。

主要输出：

- BIT：GT2001_0316\GT2001_0316.runs\impl_1\system_wrapper.bit
- ELF：GT2001_0316\GT2001_0316.sdk\GT2001_0324sdk\GT2001\Debug\GT2001.elf

SDK 唯一有效的硬件平台为：

GT2001_0316\GT2001_0316.sdk\system_wrapper_hw_platform_0

1_BIT.bat 构建成功后会把最新 BIT 同步到这个平台。不要再创建或引用
system_wrapper_hw_platform_1、_2、_3、_4，也不要使用 GT2001_0324sdk 下的旧硬件平台。

三、可选操作

- 运行 02_GENERATE_BOOT_BIN.bat 生成启动镜像：
  GT2001_0316\GT2001_0316.sdk\GT2001_0324sdk\GT2001\bootimage\BOOT.bin
- 运行 03_LOAD_JTAG.bat 通过 JTAG 下载 BIT 和 ELF。该脚本当前使用：
  D:\app\pro_app\Vivado\SDK\2018.3\bin\xsct.bat
  如果 SDK 安装位置不同，请修改脚本中的 XSCT 路径。

四、代码维护约定

- Ku_TR_SPI 的唯一 HDL/IP 源码目录：Ku_IP\ip_repo\Ku_TR_SPI_1.0
- GT2001_0316.srcs 下的同名文件是 Vivado 生成或复制的工程产物，不应作为手工修改源。
- 应用源码目录：GT2001_0316\GT2001_0316.sdk\GT2001_0324sdk\GT2001

当前版本保留板级上电、命令解析、通道状态 OR/AND 控制和 DRIVE 16 / SLEW FAST；
TX/RX 幅相写在每帧结束时产生 LDAC 动作；RX AMP2、AMP1 初始化值为 32。
RX 管脚为 RX_SDI=B22、RX_LDAC=E20。

五、四寄存器配置与硬件回读命令

- 命令：`$xxREG,reg1,reg2,reg3,reg4,dir*CS`，末尾的 `\r\n` 可带也可不带。
- `dir=1`（TX）：`reg1=PD, reg2=PA, reg3=AMP1, reg4=AMP2`
- `dir=0`（RX）：`reg1=LNA1, reg2=LNA2, reg3=AMP1, reg4=AMP2`
- 寄存器值支持十进制或 `0x` 前缀十六进制，范围均为 0~255。
- `CS` 是 `$` 与 `*` 之间所有 ASCII 字符逐字节异或后的两位十六进制值。
- 频繁手动调试时，REG 命令允许固定使用 `*00` 跳过输入校验，例如
  `$xxREG,32,32,63,63,1*00`；动态 XOR 校验方式仍然保留。
- 命令配置对应方向全部 16 颗芯片、每颗芯片 8 个对应地址，并逐地址硬件回读。
- 命令从 UART0 接收；全部 REG 应答通过与 `BEAM_UPDATE` 相同的调试输出链路发送，即 UART3。

全部 512 个回读值一致时应答：

`$xxREG,reg1,reg2,reg3,reg4,dir,OK*CS\r\n`

失败时应答：

`$xxREG,reg1,reg2,reg3,reg4,dir,FAIL,error_count,first_chip,first_addr,first_read*CS\r\n`

其中 `first_read=65535` 表示首个失败项是 SPI 超时；格式错误和任务忙分别返回
`$xxREG,ERR,FORMAT*CS\r\n` 和 `$xxREG,ERR,BUSY*CS\r\n`；校验错误返回
`$xxREG,ERR,CHECKSUM,CALC=xx,RX=xx,LEN=n*CS\r\n`，随后输出实际接收帧的
`REG_RX_RAW` 十六进制诊断行。

启动时可看到以下关键串口信息：

- KU_FINAL SERIALFIX 2026-07-23
- KU_FINAL TX init queued（启用 TX 时）
- KU_FINAL RX init queued（启用 RX 时）
- KU_FINAL beam init done
- GT1002 SATRT（当前 main.c 中保留的历史字符串）
