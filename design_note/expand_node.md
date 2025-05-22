# 节点展开/折叠功能设计

## 功能描述
实现节点的分层展示功能，默认只渲染父级节点，当点击展开按钮时切换到子节点视图。这样可以创建一个清晰的分层结构，提高复杂网络的可用性。

## 实现顺序

### 第一阶段：基础数据结构
1. 在`Node`类中添加展开状态标志
2. 添加子节点列表引用
3. 添加父节点引用
4. 添加视图层级标记

### 第二阶段：基础UI
1. 在节点Middle区域添加简单的展开/折叠按钮
2. 实现基本的按钮点击响应
3. 添加按钮的视觉样式

### 第三阶段：视图切换
1. 实现基本的视图切换逻辑
2. 添加节点展开/折叠状态切换
3. 实现子节点视图的渲染

### 第四阶段：状态管理
1. 实现节点状态的保存/恢复
2. 处理节点切换时的连接线
3. 完善节点层级关系管理

### 第五阶段：交互优化
1. 添加快捷键支持
2. 优化节点切换体验
3. 添加视觉反馈

## 编辑计划

### 1. 数据结构修改
- 在`Node`类中添加展开/折叠状态标志
- 添加子节点列表的引用
- 添加父节点引用
- 添加当前视图层级标记

```cpp
struct Node {
    bool isExpanded;        // 展开状态
    int viewLevel;          // 当前视图层级
    Node* parentNode;       // 父节点引用
    std::vector<Node*> childNodes;  // 子节点列表
    // ... 现有代码 ...
};
```

### 2. UI修改
- 在节点的Middle区域添加展开/折叠按钮
- 设计按钮的样式和交互效果
- 添加展开/折叠的动画效果（可选）

```cpp
void RenderNodeMiddle(Node* node) {
    if (!node->childNodes.empty()) {
        // 渲染展开/折叠按钮
        if (ImGui::Button(node->isExpanded ? "折叠" : "展开")) {
            ToggleNodeExpansion(node);
        }
    }
}
```

### 3. 渲染逻辑修改
- 修改`BuildNodesFromJson`函数，支持分层渲染
- 添加节点切换逻辑
- 实现节点展开/折叠时的视图切换

```cpp
Node* BuildNodesFromJson(const json& nodeJson, ImVec2 position, int depth = 0) {
    // ... 现有代码 ...
    
    // 只渲染当前层级的节点
    if (node->viewLevel == currentViewLevel) {
        // 渲染节点
    }
    
    // 处理子节点
    if (nodeJson.contains("children")) {
        for (auto& childJson : nodeJson["children"]) {
            auto childNode = BuildNodesFromJson(childJson, position, depth + 1);
            if (childNode) {
                node->childNodes.push_back(childNode);
                childNode->parentNode = node;
            }
        }
    }
}
```

### 4. 状态管理
- 添加节点展开状态的保存/恢复机制
- 处理节点切换时的连接线显示
- 维护节点层级关系

```cpp
void ToggleNodeExpansion(Node* node) {
    node->isExpanded = !node->isExpanded;
    if (node->isExpanded) {
        // 切换到子节点视图
        SwitchToChildView(node);
    } else {
        // 返回父节点视图
        SwitchToParentView(node);
    }
}
```

### 5. 交互优化
- 添加展开/折叠的快捷键支持
- 优化节点切换时的用户体验
- 添加展开/折叠状态的视觉提示

```cpp
void HandleNodeShortcuts() {
    if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
        // 切换当前选中节点的展开状态
        ToggleNodeExpansion(selectedNode);
    }
}
```

## 待讨论问题

1. 展开/折叠按钮的具体样式和位置
2. 节点切换时的动画效果需求
3. 是否需要保存节点的展开状态
4. 子节点视图的布局方式
5. 返回父节点视图的导航方式

## 实现注意事项

1. 确保节点状态切换时的数据一致性
2. 优化节点切换时的性能
3. 保持用户界面的响应性
4. 提供清晰的视觉反馈
5. 考虑大规模节点网络的情况

## 每个阶段的具体改动

### 第一阶段改动
1. 修改`Node`类定义
2. 更新节点创建逻辑
3. 添加基本的父子关系维护

### 第二阶段改动
1. 添加按钮UI组件
2. 实现按钮点击处理
3. 添加按钮样式定义

### 第三阶段改动
1. 实现视图切换函数
2. 修改节点渲染逻辑
3. 添加子节点视图处理

### 第四阶段改动
1. 添加状态序列化
2. 实现连接线处理
3. 完善层级管理

### 第五阶段改动
1. 添加快捷键处理
2. 优化切换动画
3. 添加状态提示 