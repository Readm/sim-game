#include <doctest/doctest.h>
#include "../include/node_editor.h"
#include "../include/node.h"
#include "../include/pin.h"
#include "../include/link.h"

using namespace sim::gui;

TEST_SUITE("NodeEditor") {
    TEST_CASE("初始化测试") {
        NodeEditor editor;
        CHECK(editor.Initialize());
    }

    TEST_CASE("节点创建和删除测试") {
        NodeEditor editor;
        REQUIRE(editor.Initialize());

        // 创建节点
        auto node = editor.CreateNode("测试节点", ImVec2(100, 100));
        REQUIRE(node != nullptr);
        CHECK_EQ(node->GetName(), "测试节点");
        CHECK_EQ(node->GetType(), Node::Type::Blueprint);

        // 删除节点
        editor.DeleteNode(node);
    }

    TEST_CASE("引脚创建测试") {
        NodeEditor editor;
        REQUIRE(editor.Initialize());

        auto node = editor.CreateNode("测试节点", ImVec2(100, 100));
        REQUIRE(node != nullptr);

        // 添加输入引脚
        node->AddInputPin("输入", ed::PinKind::Input);
        
        // 添加输出引脚
        node->AddOutputPin("输出", ed::PinKind::Output);

        // 验证引脚数量
        CHECK_EQ(node->GetInputPins().size() + node->GetOutputPins().size(), 2);
    }

    TEST_CASE("连接创建和删除测试") {
        NodeEditor editor;
        REQUIRE(editor.Initialize());

        // 创建两个节点
        auto node1 = editor.CreateNode("节点1", ImVec2(100, 100));
        auto node2 = editor.CreateNode("节点2", ImVec2(300, 100));
        REQUIRE(node1 != nullptr);
        REQUIRE(node2 != nullptr);

        // 添加引脚
        node1->AddOutputPin("输出", ed::PinKind::Output);
        node2->AddInputPin("输入", ed::PinKind::Input);

        // 创建连接
        auto startPin = node1->GetOutputPins()[0];
        auto endPin = node2->GetInputPins()[0];
        auto link = editor.CreateLink(startPin, endPin);
        REQUIRE(link != nullptr);

        // 删除连接
        editor.DeleteLink(link);
    }
} 