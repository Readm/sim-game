#include "doctest.h"
#include "flatbuffers/flatbuffers.h"
#include <iostream>

TEST_CASE("FlatBuffers: Create and verify a simple buffer with a string") {
    // Create a FlatBufferBuilder
    flatbuffers::FlatBufferBuilder builder;

    // Create a simple buffer with a string
    auto str = builder.CreateString("Hello, FlatBuffers!");
    builder.Finish(str);

    // Access the buffer
    const char* buffer = reinterpret_cast<const char*>(builder.GetBufferPointer());
    size_t size = builder.GetSize();

    // Verify the buffer size
    CHECK(size > 0);

    // Verify the string
    auto retrieved_str = flatbuffers::GetRoot<flatbuffers::String>(buffer);
    REQUIRE(retrieved_str != nullptr);
    CHECK(std::string(retrieved_str->c_str()) == "Hello, FlatBuffers!");
}
