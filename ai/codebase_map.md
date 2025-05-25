# 代码库地图

## 项目结构概览

```
sim-game/
├── src/                    # 源代码目录
│   ├── sim/               # 模拟引擎核心
│   └── gui/               # GUI界面代码
├── ai/           # 设计文档
├── external/              # 外部依赖
├── build/                 # 构建输出
├── data/                  # 数据文件
├── docs/                  # 生成的文档
└── CMakeLists.txt         # 构建配置
```

## 核心模块详解

### 1. 模拟引擎 (`src/sim/`)

#### 头文件 (`src/sim/include/`)
- **`common.h`** - 编译配置和全局定义
  - 编译等级宏定义
  - 统一的错误报告等级
  - 全局常量定义

- **`type.h`** - 类型系统核心
  - `NodeID` 和 `PacketID` 类型定义
  - 序列化方法枚举（JSON、BINARY、PROTOBUF）
  - 并行化方法枚举（NONE、OPENMP、THREAD_POOL）
  - 模拟模式枚举（FASTEST、STEP、TRACE）
  - 类型注册系统实现

- **`packet.h`** - 数据包系统
  - `Packet` 抽象基类
  - `VoidPacket` - 基础空包实现
  - `InfoPacket` - 信息包实现
  - 包的序列化/反序列化接口

- **`node.h`** - 节点系统核心
  - `Node` 抽象基类
  - 核心模拟逻辑（Tick/Tock）
  - 子节点管理
  - 缓冲区管理
  - 序列化/反序列化实现

- **`port.h`** - 端口系统
  - `InputPort` 和 `OutputPort` 类
  - 端口连接管理
  - 数据流控制
  - 端口容量限制

- **`network.h`** - 网络顶层节点
  - `Network` 类 - 顶层容器节点
  - 网络级别的模拟控制
  - 全局状态管理

- **`thread_pool.h`** - 线程池实现
  - 自定义线程池类
  - 任务队列管理
  - 线程安全机制

- **`producer_consumer.h`** - 生产者消费者模型
  - `ProducerNode` - 数据生产节点
  - `ConsumerNode` - 数据消费节点
  - 基础测试节点实现

- **`network_factory.h`** - 网络工厂
  - 预定义网络创建
  - 网络配置管理
  - 网络序列化/反序列化

- **`server.h`** - 服务器实现
  - HTTP服务器类
  - WebSocket服务器类
  - API接口定义
  - 模拟控制接口

#### 实现文件 (`src/sim/`)
- 对应头文件的具体实现
- 核心算法实现
- 平台相关代码

#### 单元测试 (`src/sim/unittest/`)
- 使用doctest框架
- 每个核心类都有对应测试
- 性能测试和压力测试
- 序列化测试

### 2. GUI界面 (`src/gui/`)

#### 核心组件
- **主界面管理**
  - 窗口创建和管理
  - ImGui初始化和渲染循环
  - 事件处理

- **网络可视化**
  - 基于imgui-node-editor
  - 节点渲染和交互
  - 连接线绘制
  - 缩放和平移

- **节点编辑器**
  - 节点创建和删除
  - 属性编辑界面
  - 连接管理
  - 上下文菜单

- **通信模块**
  - HTTP客户端实现
  - WebSocket客户端实现
  - 与后端的数据同步
  - 实时状态更新

- **状态显示**
  - 节点状态可视化
  - 缓冲区状态显示
  - 端口状态指示
  - 性能监控界面

## 特殊节点实现

### 基础节点类型
- **`Network`** - 顶层网络容器
  - 位置：`src/sim/include/network.h`
  - 功能：管理所有顶层模块，只生成VoidPacket

- **`FIFO`** - 先进先出队列节点
  - 功能：单输入单输出，简单数据传递
  - 用途：基础数据流控制

- **`ProducerNode`** - 生产者节点
  - 位置：`src/sim/include/producer_consumer.h`
  - 功能：定期生成VoidPacket
  - 用途：测试和数据源

- **`ConsumerNode`** - 消费者节点
  - 位置：`src/sim/include/producer_consumer.h`
  - 功能：消费输入数据包
  - 用途：测试和数据汇聚

## 外部依赖 (`external/`)

### 主要依赖库
- **ImGui** - 即时模式GUI库
- **imgui-node-editor** - 节点编辑器扩展
- **nlohmann/json** - JSON序列化库
- **doctest** - 单元测试框架
- **OpenGL** - 图形渲染
- **GLFW** - 窗口管理

## 构建系统

### CMake配置
- **主CMakeLists.txt** - 项目根配置
- **子模块配置** - 各模块独立配置
- **依赖管理** - 外部库集成
- **测试配置** - 单元测试集成

### 构建目标
- **sim_engine** - 模拟引擎库
- **gui_app** - GUI应用程序
- **unit_tests** - 单元测试可执行文件
- **integration_tests** - 集成测试

## 数据文件 (`data/`)

### 配置文件
- 网络配置JSON文件
- 节点模板文件
- 默认参数配置

### 测试数据
- 单元测试数据
- 性能测试场景
- 示例网络配置

## 文档系统 (`docs/`)

### 生成文档
- **Doxygen配置** - API文档生成
- **用户手册** - 使用说明文档
- **开发指南** - 开发者文档

## 关键文件说明

### 配置文件
- **`Doxyfile`** - Doxygen文档生成配置
- **`.gitmodules`** - Git子模块配置
- **`imgui.ini`** - ImGui界面配置
- **`.gitignore`** - Git忽略文件配置

### 运行时文件
- **`serialized_node.json`** - 序列化的节点状态
- **构建产物** - 在build/目录下

## 开发工作流

### 新增节点类型
1. 在`src/sim/include/`添加头文件
2. 在`src/sim/`添加实现文件
3. 在`src/sim/unittest/`添加测试
4. 更新`network_factory.h`支持创建
5. 在GUI中添加可视化支持

### 新增功能模块
1. 设计接口和数据结构
2. 实现核心逻辑
3. 添加单元测试
4. 集成到主系统
5. 更新文档

### 调试和测试
1. 使用单元测试验证功能
2. 通过GUI进行可视化调试
3. 使用序列化测试验证状态一致性
4. 性能测试验证并行化效果 