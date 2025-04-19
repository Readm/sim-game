# Blueprints Example 分析

## 项目结构

blueprints-example 是一个基于 ImGui 和 ImGui-Node-Editor 的蓝图编辑器示例。主要包含以下文件：

- `blueprints-example.cpp`: 主程序文件，包含编辑器的主要逻辑
- `utilities/`: 工具类目录
  - `builders.cpp/h`: 节点构建相关工具
  - `drawing.cpp/h`: 绘图相关工具
  - `widgets.cpp/h`: 自定义控件相关工具

## 核心设计

### 1. 节点系统

示例实现了一个完整的节点系统，包含以下核心组件：

- `Node`: 节点基类
  - 包含输入输出引脚
  - 支持多种节点类型（Blueprint、Simple、Tree、Comment、Houdini）
  - 具有位置、大小、颜色等属性

- `Pin`: 引脚类
  - 支持多种数据类型（Flow、Bool、Int、Float、String等）
  - 分为输入和输出两种类型
  - 具有ID、名称、类型等属性

- `Link`: 连接类
  - 连接两个引脚
  - 具有ID、起始引脚ID、结束引脚ID等属性

### 2. 编辑器初始化

编辑器初始化过程：

1. 创建编辑器配置（ed::Config）
2. 设置配置文件路径
3. 设置节点状态保存/加载回调
4. 创建编辑器上下文
5. 设置当前编辑器
6. 创建初始节点并设置位置

### 3. 渲染循环

渲染循环主要包含以下步骤：

1. 开始新帧
2. 绘制节点
3. 绘制连接
4. 处理节点创建
5. 处理连接创建
6. 处理删除操作
7. 显示上下文菜单
8. 结束帧

### 4. 交互功能

- 节点拖拽
- 连接创建
- 节点删除
- 连接删除
- 上下文菜单
- 节点状态保存/恢复
- 缩放和平移

## ImGui 集成

1. 初始化：
```cpp
// 创建 ImGui 上下文
m_ImGuiContext = ImGui::CreateContext();

// 初始化 ImGui GLFW 和 OpenGL3 后端
ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
ImGui_ImplOpenGL3_Init("#version 130");
```

2. 渲染循环：
```cpp
// 开始新帧
ImGui_ImplOpenGL3_NewFrame();
ImGui_ImplGlfw_NewFrame();
ImGui::NewFrame();

// 绘制编辑器
ed::Begin("Node Editor");

// 结束帧
ed::End();
ImGui::Render();
ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
```

## 使用建议

1. 节点设计：
   - 合理规划节点类型和引脚类型
   - 注意节点状态的保存和恢复
   - 考虑节点的可扩展性

2. 性能优化：
   - 避免频繁的节点创建和删除
   - 合理使用节点缓存
   - 优化渲染逻辑

3. 交互设计：
   - 提供清晰的视觉反馈
   - 实现合理的撤销/重做机制
   - 考虑用户习惯和操作效率 