

# 源码构成

+ `src/sim`目录下是模拟部分的代码，可以独立运行
    + `src/sim/include`是该部分代码的头文件位置
      + `src/sim/include/common.h`定义了编译相关的宏，包括编译等级，统一的报错等级等
      + `src/sim/include/type.h`定义了Node和Packet的ID类型
      + `src/sim/include/packet.h`定义了Packet类型
    + `src/sim`是对应的实现
    + `src/sim/unittest`是对应的单元测试
      + 单元测试使用doctest框架
+ src/gui目录下是GUI代码，可以用于查看和执行模拟
