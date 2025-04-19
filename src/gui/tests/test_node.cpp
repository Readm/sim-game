#include <doctest/doctest.h>
#include "../include/node.h"
#include "../include/pin.h"
#include "test_utils.h"

using namespace sim::gui;
using namespace sim::gui::test;

TEST_SUITE("Node") {
    TEST_CASE("构造函数测试") {
        TestEnvironment env;
        
        auto node = std::make_shared<Node>("测试节点", Node::Type::Blueprint, ImVec2(100, 100));
        
        CHECK_EQ(node->GetName(), "测试节点");
        CHECK_EQ(node->GetType(), Node::Type::Blueprint);
        CHECK_EQ(node->GetPosition().x, 100);
        CHECK_EQ(node->GetPosition().y, 100);
    }

    TEST_CASE("引脚添加测试") {
        TestEnvironment env;
        
        auto node = std::make_shared<Node>("测试节点", Node::Type::Blueprint, ImVec2(100, 100));
        
        // 添加输入引脚
        node->AddInputPin("输入", ed::PinKind::Input);
        CHECK_EQ(node->GetInputPins().size(), 1);
        CHECK_EQ(node->GetInputPins()[0]->GetName(), "输入");
        CHECK_EQ(node->GetInputPins()[0]->GetKind(), ed::PinKind::Input);
        
        // 添加输出引脚
        node->AddOutputPin("输出", ed::PinKind::Output);
        CHECK_EQ(node->GetOutputPins().size(), 1);
        CHECK_EQ(node->GetOutputPins()[0]->GetName(), "输出");
        CHECK_EQ(node->GetOutputPins()[0]->GetKind(), ed::PinKind::Output);
    }

    TEST_CASE("位置设置测试") {
        TestEnvironment env;
        
        auto node = std::make_shared<Node>("测试节点", Node::Type::Blueprint, ImVec2(100, 100));
        
        // 设置新位置
        node->SetPosition(ImVec2(200, 200));
        CHECK_EQ(node->GetPosition().x, 200);
        CHECK_EQ(node->GetPosition().y, 200);
    }
} 