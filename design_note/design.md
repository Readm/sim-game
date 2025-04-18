# 源码构成

+ `src/sim`目录下是模拟部分的代码，可以独立运行
    + `src/sim/include`是该部分代码的头文件位置
      + `src/sim/include/common.h`定义了编译相关的宏，包括编译等级，统一的报错等级等
      + `src/sim/include/type.h`
        + 定义了Node和Packet的ID类型
        + 定义了不同的序列化方法（JSON、BINARY、PROTOBUF）
        + 定义了不同的并行化方法（NONE、OPENMP、THREAD_POOL）
        + 定义了模拟模式（FASTEST、STEP、TRACE）
        + 提供类型注册系统
      + `src/sim/include/packet.h`定义了Packet类型及其派生类
      + `src/sim/include/node.h`定义了Node类型及核心模拟逻辑
      + `src/sim/include/port.h`定义了输入输出端口系统
      + `src/sim/include/network.h`定义了Network顶层节点类型
      + `src/sim/include/thread_pool.h`定义了线程池实现
      + `src/sim/include/producer_consumer.h`定义了生产者-消费者模型
      + `src/sim/include/network_factory.h`提供网络创建和管理的工厂类
      + `src/sim/include/server.h`提供HTTP和WebSocket服务器实现
    + `src/sim`是对应的实现
    + `src/sim/unittest`是对应的单元测试
      + 单元测试使用doctest框架
+ `src/gui`目录下是GUI代码，可以用于查看和执行模拟
    + 使用imgui和imgui-node-editor实现可视化界面
    + 提供网络可视化、节点编辑和模拟控制功能
    + 通过HTTP和WebSocket与服务器通信
    + 实现了多种测试场景和集成测试

# 基础架构

+ 模拟的基本模型是Node和Packet模型
  + Node和Packet都可以被在任意时刻被序列化和反序列化，目前开发阶段使用json格式，后续支持BINARY和PROTOBUF格式
+ Packet 是模拟数据的基础单位，没有时间概念
  + Packet是一个抽象类
  + 一个Packet必然有一个src_node_id，代表它是由哪个节点生成的，如果没有指定，那么就是NetWork，NetWork是最顶层的节点
  + Packet没有独立的构造函数，一定由一个Node节点通过Spawn函数来生成函数。每个Node只能生成一个种类的Packet。
  + 一个Node生成的PacketID不会重复
  + Packet有playloads，可以用PacketID引用其他Packet（而不是指针），需要用static函数getPayloadByID(PayloadID)来获取
+ Node 是一个模拟模块的基础单位
  + Node是一个抽象类
  + 模块的可以有多个child_node
  + 模块中有一个buffer，内部可以保存若干个Packet
  + 在序列化时，child_node和buffer中都自动调用Packet和Node的序列化函数，加入到当前Node序列化的内容，反序列化也一样
  + Node有若干个输入/输出Port，不同节点的输入和输出Port可以相互链接（可以多对一或者一对多），每个Port只能接收同一种包，每个输入Port可以有一定的数量上限
    + 每一个Tick，Node可以查看输出Port对应的外部输入Port是否达到了数量上限，用来记录是否后面的Tock是否需要产出新的输出
    + 每一个Tock，如果Node需要产出新的输出，那么产出，并放置到对应的输入Port中。
+ ID系统
  + TypeID系统：每个Node和Packet类都有一个固定的TypeID，可以使用type.h提供的Hash函数来通过名字生成，也可以直接指定。
    + 系统中将在程序载入时，检查这些TypeID是否有重复（使用static在程序初始化执行代码来查重）
+ 时间系统
  + 时间类型为TickTock(uint_64_t)，每个Node的实例有一个TickTock，使用Node的Tick和Tock虚函数来允许派生类定义自己在Tick和Tock时应该做什么
  + Tick函数不应该改变Node的模拟状态（buffer，port等），只是用于其他相关节点读取自身的状态，同时在这个状态下也需要读取其他相关节点读取自身状态。
  + Tock函数不应该再观察其他节点，更新当前本节点应该执行的更新操作
  + 一个节点的所有子节点的Tick和所有子节点的Tock可以并行执行
  + 当在Debug模式下时，每一拍的Tick都会在执行前和执行后序列化一次，检查是否序列化结果一致。
  + 当在Debug模式下时，每一拍的Tock都会尝试在一个新的进程中反序列化一个节点并执行Tock，并序列化到当前进程，来确认它确实可以独立运行，对其他节点没有依赖。

## 序列化系统
+ 系统支持三种序列化方法：
  + JSON：基于nlohmann::json库实现，用于开发和调试
  + BINARY：二进制格式，用于高性能序列化（待实现）
  + PROTOBUF：基于Google Protocol Buffers，用于跨语言兼容（待实现）
+ 序列化特性：
  + 所有Node和Packet都实现了serialize和deserialize方法
  + 支持完整的状态保存和恢复
  + 序列化内容包括节点ID、类型信息、内部状态等
  + 支持嵌套对象的序列化（子节点、缓冲区等）

## 并行化系统
+ 系统支持三种并行化方法：
  + NONE：无并行处理，顺序执行
  + OPENMP：使用OpenMP实现并行化（待实现）
  + THREAD_POOL：使用自定义线程池实现并行化
+ 并行化特性：
  + 通过ThreadPool实现任务并行执行
  + 支持子节点的Tick和Tock并行处理
  + 可以根据硬件配置自动调整线程数
  + 提供安全的任务提交和结果获取机制

## 网络服务器
+ 服务器功能：
  + 提供HTTP API接口控制模拟
  + 支持WebSocket实时状态更新
  + 提供模拟控制（启动、停止、单步、重置）
  + 支持网络配置的加载和保存
+ 服务器组成：
  + Server类：处理HTTP和WebSocket请求
  + SimulationEngine类：实际执行模拟计算

## 特殊Node和Packet

+ Packet
  + VoidPacket：没有任何除了Packet基础功能以外功能的Packet
  + InfoPacket：包含了一个信息的Packet，用于记录各种信息
+ Node
  + Network: Top Level Node，代表整个网络，即，他的Child是所以模拟的顶层模块，它只能生成VoidPacket。
  + FIFO：仅能接收一种Packet，仅有一个输入Port和输出Port，每一个TickTock仅仅获取输入Port的Packet，并向后输出。
  + ProducerNode：生产者节点，每个Tick-Tock周期生成一个VoidPacket并发送到输出端口
  + ConsumerNode：消费者节点，每个Tick-Tock周期从输入端口读取数据包

## 工厂类
+ NetworkFactory：
  + 提供预定义网络的创建方法
  + 支持网络的序列化和反序列化
  + 简化网络配置过程

## GUI系统
+ 特性：
  + 基于imgui和imgui-node-editor实现
  + 提供可视化的网络编辑器
  + 支持节点创建、连接和配置
  + 实时显示模拟状态和数据流
  + 通过HTTP和WebSocket与服务器通信
+ 组件：
  + NetworkViewer：网络可视化组件
  + HttpClient：HTTP客户端
  + NetworkClient：网络客户端，处理WebSocket连接
  + NodeGUI：节点GUI表示