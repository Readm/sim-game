# GUI节点测试计划

## 测试目标
生成一个包含多接口节点的Network的JSON序列化文件，用于GUI测试。

## 测试文件结构
- 测试文件：`src/sim/unittest/gui_node_test.cpp`
- 输出JSON：`src/sim/unittest/gui_node_test.json`

## 测试场景设计
```
Network
├── MultiInputNode (2个输入端口，1个输出端口)
└── MultiOutputNode (1个输入端口，2个输出端口)
```

## 测试内容

### 1. 节点基本属性
- 节点ID
- 节点类型
- 节点名称

### 2. 端口配置
- 输入端口数量
- 输出端口数量
- 端口ID
- 端口类型

### 3. 连接关系
- 多输入到单输出的连接
- 单输入到多输出的连接

## 测试步骤

1. 创建Network节点
2. 创建MultiInputNode和MultiOutputNode
3. 配置节点属性
4. 建立节点连接
5. 序列化为JSON
6. 保存到文件

## 预期结果
生成一个包含完整节点信息和连接关系的JSON文件，供GUI测试使用。 