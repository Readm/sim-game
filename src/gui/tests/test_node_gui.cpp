#include <doctest/doctest.h>
#include "../include/node_gui.h"
#include "test_utils.h"
#include <nlohmann/json.hpp>
#include <iostream>

using namespace sim::gui;
using namespace sim::gui::test;
using json = nlohmann::json;

TEST_SUITE("NodeGUI") {
    TEST_CASE("初始化测试") {
        std::cout << "开始初始化测试..." << std::endl;
        TestEnvironment env;
        NodeGUI gui;
        CHECK(gui.Initialize());
        
        // 显示测试结果
        env.ShowVisualForSeconds(2.0f);
        std::cout << "初始化测试完成" << std::endl;
    }

    TEST_CASE("从JSON加载节点测试") {
        std::cout << "开始JSON加载测试..." << std::endl;
        
        TestEnvironment env;
        NodeGUI gui;
        REQUIRE(gui.Initialize());

        // 创建测试JSON
        json j;
        j["nodes"] = json::array();
        
        json node;
        node["name"] = "测试节点";
        node["position"] = json::object();
        node["position"]["x"] = 100.0f;
        node["position"]["y"] = 100.0f;
        
        node["inputs"] = json::array();
        json input;
        input["name"] = "输入1";
        node["inputs"].push_back(input);
        
        node["outputs"] = json::array();
        json output;
        output["name"] = "输出1";
        node["outputs"].push_back(output);
        
        j["nodes"].push_back(node);

        std::string jsonStr = j.dump(2);
        std::cout << "测试JSON数据：\n" << jsonStr << std::endl;

        // 加载JSON
        bool loadResult = gui.LoadFromJson(jsonStr);
        std::cout << "JSON加载结果：" << (loadResult ? "成功" : "失败") << std::endl;
        CHECK(loadResult);

        // 验证节点创建
        auto& editor = gui.GetNodeEditor();
        std::cout << "开始渲染测试..." << std::endl;
        REQUIRE_NOTHROW(editor.Render());
        
        // 显示测试结果
        env.ShowVisualForSeconds(3.0f);
        std::cout << "渲染测试完成" << std::endl;
    }
} 