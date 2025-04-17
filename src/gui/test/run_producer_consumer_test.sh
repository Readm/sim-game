#!/bin/bash

# 设置工作目录为项目根目录
cd "$(dirname "$0")/../../../"

# 确保项目已经编译过，如果需要重新编译可以使用下面命令
echo "正在编译项目..."
mkdir -p build
cd build
cmake ..
make -j4 producer_consumer_test

# 运行测试程序
echo ""
echo "======================="
echo "正在启动生产者-消费者网络测试..."
echo "======================="
./src/gui/producer_consumer_test $@

# 完成
echo ""
echo "测试程序已退出" 