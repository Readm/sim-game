

# 源码构成

+ `src/sim`目录下是模拟部分的代码，可以独立运行
    + `src/sim/include`是该部分代码的头文件位置
      + `src/sim/include/common.h`定义了编译相关的宏，包括编译等级，统一的报错等级等
      + `src/sim/include/type.h`
        + 定义了Node和Packet的ID类型
        + 定义了不同的序列化方法
        + 定义了不同的并行化方法
      + `src/sim/include/packet.h`定义了Packet类型
      + `src/sim/include/node.h`定义了Node类型
    + `src/sim`是对应的实现
    + `src/sim/unittest`是对应的单元测试
      + 单元测试使用doctest框架
+ src/gui目录下是GUI代码，可以用于查看和执行模拟


# 基础架构

+ 模拟的基本模型是Node和Packet模型
  + Node和Packet都可以被在任意时刻被序列化和反序列化，目前开发阶段使用json格式，后续支持更多
+ Packet 是模拟数据的基础单位，没有时间概念
  + Packet是一个抽象类
  + 一个Packet必然有一个src_node_id，代表它是由哪个节点生成的，如果没有指定，那么就是NetWork，NetWork是最顶层的节点
  + 一个Node生成的PacketID不会重复
  + Packet有playloads，可以用PacketID引用其他Packet（而不是指针），需要用static函数getPayloadByID(PayloadID)来获取
+ Node 是一个模拟模块的基础单位
  + Node是一个抽象类
  + 模块的可以有多个child_node
  + 模块中有一个buffer，内部可以保存若干个Packet
  + 在序列化时，child_node和buffer中都自动调用Packet和Node的序列化函数，加入到当前Node序列化的内容，反序列化也一样
+ ID系统
  + TypeID系统：每个Node和Packet类都有一个固定的TypeID，可以使用type.h提供的Hash函数来通过名字生成，也可以直接指定。
    + 系统中将在程序载入时，检查这些TypeID是否有重复（使用static在程序初始化执行代码来查重）


## 特殊Node和Packet

+ Node
  + Network: Top Level Node，代表整个网络，即，他的Child是所以模拟的顶层模块
+ Packet
  + VoidPacket：没有任何除了Packet基础功能以外功能的Packet
  + InfoPacket：包含了一个信息的Packet，用于记录各种信息