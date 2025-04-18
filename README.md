# sim-game：可视化模拟框架

## 愿景和目标

这个世界并不缺少两类东西：

+ 各种计算机体系结构和网络的模拟器：GEM5，QEMU等
+ 各种与计算机相关的游戏：ShenzhenIO，人力资源机器，Hacknet等等
  
其实流水线类的游戏和部分体系架构的模拟器逻辑非常吻合：都是特定的数据包，在某些通路上，做某些处理。看看[异形工厂](https://store.steampowered.com/app/2162800/2/)，这玩意和SoC模拟不是一个逻辑吗？

本项目旨在建立一个桥梁，让计算机体系结构的模拟器和游戏打通，主要目标：

1. **提供直观的可视化界面**：让复杂的模拟过程可视化，便于理解各种概念（如活锁、死锁、阻塞等）
2. **提供精确、简单、有用的抽象**：确保模拟结果可靠，同时保持足够的表达能力
3. **支持高效并行处理**：充分利用多线程、GPU加速等技术提高模拟速度
4. **模拟逻辑和交互界面解耦**：在不需要游戏逻辑时，可以将所有算力用于模拟计算

## 系统架构

系统基于Tick-Tock机制实现，主要由以下部分组成：

### 核心模拟引擎 (src/sim)

+ **Node和Packet模型**：
  + Node是模拟模块的基础单位，可以有多个子节点
  + Packet是模拟数据的基础单位，由节点生成和处理
  + 支持在任意时刻序列化和反序列化

+ **端口系统**：
  + 节点间通过输入/输出端口连接
  + 支持多对一或一对多的连接
  + 每个端口只接收特定类型的数据包

+ **时间系统**：
  + 使用Tick-Tock机制同步各节点
  + Tick阶段读取状态但不修改
  + Tock阶段更新状态
  + 支持子节点并行执行

+ **序列化系统**：
  + 支持JSON、BINARY和PROTOBUF三种方式
  + 可完整保存和恢复系统状态

+ **并行化系统**：
  + 支持无并行、OpenMP和线程池三种方式
  + 可根据硬件自动调整线程数

+ **网络服务器**：
  + 提供HTTP API和WebSocket接口
  + 支持模拟控制和状态监控

### 可视化界面 (src/gui)

+ 基于imgui和imgui-node-editor实现
+ 提供网络可视化、节点编辑和模拟控制
+ 通过HTTP和WebSocket与服务器通信
+ 支持实时显示模拟状态和数据流

## 主要组件

### 核心类型

1. **Node类型**
   - Network: 顶层节点，代表整个网络
   - FIFO: 简单的先进先出节点
   - ProducerNode: 生产者节点，周期性生成数据包
   - ConsumerNode: 消费者节点，从输入端口读取数据包

2. **Packet类型**
   - VoidPacket: 基础空数据包
   - InfoPacket: 携带信息的数据包

3. **工厂类**
   - NetworkFactory: 提供预定义网络的创建方法

4. **GUI组件**
   - NetworkViewer: 网络可视化组件
   - HttpClient: HTTP客户端
   - NetworkClient: 网络客户端
   - NodeGUI: 节点GUI表示

## 适用场景

本项目适用于以下场景的模拟：

- 简单的流水线CPU
- TCP AIMD 拥塞控制模拟
- 其他需要可视化理解的网络和流程模拟

## 构建与运行

### 前置依赖

项目依赖以下第三方库，大部分作为git子模块包含在仓库中：

- [doctest](https://github.com/doctest/doctest.git) - 单元测试框架
- [cpp-httplib](https://github.com/yhirose/cpp-httplib.git) - HTTP/WebSocket客户端和服务器库
- [nlohmann/json](https://github.com/nlohmann/json.git) - JSON处理库
- [imgui-node-editor](https://github.com/thedmd/imgui-node-editor.git) - 基于ImGui的节点编辑器
- OpenGL - 图形渲染
- GLFW - 窗口和输入处理

系统依赖（需要单独安装）：
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential cmake libglfw3-dev libgl1-mesa-dev

# CentOS/RHEL
sudo yum install -y gcc-c++ cmake glfw-devel mesa-libGL-devel

# macOS (使用Homebrew)
brew install cmake glfw
```

### 获取源码

```bash
# 克隆仓库及其子模块
git clone --recursive https://github.com/yourusername/sim-game.git
cd sim-game

# 如果你已经克隆了仓库但没有子模块，可以执行
git submodule update --init --recursive
```

### 构建项目

```bash
# 创建并进入构建目录
mkdir -p build
cd build

# 配置
cmake ..

# 编译
cmake --build . --parallel $(nproc)
```

### 运行

```bash
# 运行GUI应用
./src/gui/sim_network_viewer

# 如果只想运行独立的模拟引擎
./src/sim/some_sim_executable  # 具体可执行文件名
```

## 测试

项目包含多种测试类型，包括单元测试、集成测试和特定功能测试。

### 运行所有测试

```bash
# 在构建目录中
ctest

# 或者
make test
```

### 运行特定测试

```bash
# 模拟引擎测试
./src/sim/server_test
./src/sim/sim_network_test

# GUI测试
./src/gui/sim_network_viewer_test

# 集成测试
./src/gui/network_integration_test
./src/gui/websocket_test

# 功能测试
./src/gui/producer_consumer_test
./src/gui/node_gui_test
```

### 调试构建

```bash
# 使用Debug配置构建
mkdir -p build_debug
cd build_debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --parallel $(nproc)
```

## 开发文档

请参考各个模块的详细文档：

- [设计文档](./design_note/design.md)
- [项目愿景](./design_note/readm.md)

# 临时启动html server查看
```
cd docs/html && python3 -m http.server 8000
```