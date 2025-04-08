# ImGui Node Editor Demo

这是一个基于 [imgui-node-editor](https://github.com/thedmd/imgui-node-editor) 的示例项目。

## 依赖项

- CMake 3.15+
- C++17 兼容的编译器
- Google Test (用于单元测试)

## 构建步骤

1. 克隆仓库并初始化子模块：
```bash
git clone <repository-url>
cd imgui-node-editor-demo
git submodule update --init --recursive
```

2. 创建构建目录并配置项目：
```bash
mkdir build
cd build
cmake ..
```

3. 构建项目：
```bash
cmake --build .
```

## 运行

构建完成后，可执行文件将位于 `build` 目录下：

```bash
./imgui_node_editor_demo
```

## 运行测试

```bash
./imgui_node_editor_demo_tests
```

## 项目结构

- `src/` - 源代码目录
- `tests/` - 测试代码目录
- `external/` - 外部依赖（包括 imgui-node-editor）

## 功能特性

- 基本的节点编辑器界面
- 可创建和连接节点
- 支持节点拖拽
- 支持缩放和平移
- 基本的单元测试

## 使用说明

1. 运行程序后，您将看到一个包含两个示例节点的窗口
2. 可以使用鼠标拖拽节点
3. 使用鼠标滚轮进行缩放
4. 按住鼠标中键可以平移视图

## 开发说明

要添加新的节点类型，请修改 `src/main.cpp` 文件中的 `OnFrame` 方法。

## 许可证

本项目使用 MIT 许可证。
