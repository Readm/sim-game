#include "doctest/doctest.h"
#include "type.h"

TEST_CASE("TypeID Generation") {
    using namespace sim;

    // 测试TypeID生成
    TypeID id1 = generateTypeID("TestType1");
    TypeID id2 = generateTypeID("TestType2");
    TypeID id3 = generateTypeID("TestType1");

    CHECK(id1 != id2);
    CHECK(id1 == id3);
}

TEST_CASE("Type Registry") {
    using namespace sim;

    // 测试类型注册
    TypeID id1 = generateTypeID("TestType1");
    TypeID id2 = generateTypeID("TestType2");

    CHECK(TypeRegistry::getInstance().registerType(id1, "TestType1"));
    CHECK(TypeRegistry::getInstance().registerType(id2, "TestType2"));
    
    // 测试重复注册
    CHECK(TypeRegistry::getInstance().registerType(id1, "TestType1")); // 相同类型可以重复注册
    CHECK_FALSE(TypeRegistry::getInstance().registerType(id1, "DifferentName")); // 不同名称不能注册相同ID

    // 测试类型名称获取
    CHECK(TypeRegistry::getInstance().getTypeName(id1) == "TestType1");
    CHECK(TypeRegistry::getInstance().getTypeName(id2) == "TestType2");
    CHECK(TypeRegistry::getInstance().getTypeName(0) == "UNKNOWN"); // 测试未知类型
} 