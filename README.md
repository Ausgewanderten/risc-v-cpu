该项目源自于"一生一芯"工程项目

此仓库主要用于记录我的学习代码与过程

目录结构

| 路径 | 内容 |
| --- | --- |
| [`minirv_npc/`](minirv_npc/) | MiniRV CPU RTL、SoC 仿真驱动及相关测试脚本 |
| [`ysyxSoC/`](ysyxSoC/) | SoC 集成环境、外设和 MiniRV 预生成顶层 |
| [`abstract-machine/`](abstract-machine/) | Abstract Machine 抽象运行环境、klib 和平台构建脚本 |
| [`nemu/`](nemu/) | NEMU 教学模拟器工程 |
| [`am-kernels/`](am-kernels/) | AM 测试、示例内核和基准程序 |
| [`riscv-tests/`](riscv-tests/) / [`riscv-tests-am/`](riscv-tests-am/) | RISC-V 测试相关工程 |
| [`nvboard/`](nvboard/) | NVBoard 虚拟板卡及配套资源 |
| [`npc/`](npc/) / [`npc1/`](npc1/) | 使用 Verilator 和 NVBoard 的数字电路实验 |
| [`experiment/`](experiment/) | 编码器、数码管显示等数字电路练习 |
| [`compile-test/`](compile-test/) / [`compile-test1/`](compile-test1/) | 编译、汇编、模拟器和 AM 应用相关练习 |
| [`programfile/`](programfile/) | 小型 C 程序练习 |
| [`archbench/`](archbench/) | 体系结构基准程序及输入数据 |
| [`fceux-am/`](fceux-am/) | AM 平台上的 FCEUX 相关工程

CPU 与仿真入口

[`minirv_npc/vsrc/`](minirv_npc/vsrc/) 中的主要模块如下：

| 文件 | 作用 |
| --- | --- |
| `ysyx_00000000.v` | CPU 顶层，连接取指、译码、执行、寄存器堆和访存模块 |
| `ifu.v` | 取指与执行流程控制 |
| `idu.v` | 指令译码及立即数生成 |
| `exu.v` | 执行单元 |
| `RegisterFile.v` | 寄存器堆 |
| `lsu.v` | 数据访存接口 |
| `imem.v` / `dmem.v` | 通过 DPI-C 访问存储器的模块；当前 SoC 构建以顶层实际连接为准 |

当前译码代码包含 `addi`、`add`、`lui`、`jalr`、`lw`、`sw`、`lbu`、`sb` 和 `ebreak` 的识别逻辑。这是对现有代码的描述，不代表指令行为已经通过完整验证。

CPU 顶层模块名为 `ysyx_00000000`。SoC 仿真顶层为 `SimTop`，定义在 `ysyxSoC/ready-to-run/minirv/ElaborateTop.v` 中，C++ 仿真入口为 [`minirv_npc/csrc/test.cpp`](minirv_npc/csrc/test.cpp)。
