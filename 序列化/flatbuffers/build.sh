#!/bin/bash


if [ ! -d "./master.zip" ]; then
	wget https://github.com/google/flatbuffers/archive/master.zip
fi
#解压源代码
rm -rf src
unzip master.zip
mv flatbuffers-master src

#新建编译目录
mkdir -p build
cd build
cmake ../src -G"Unix Makefiles" -DFLATBUFFERS_BUILD_SHAREDLIB=ON -DFLATBUFFERS_INSTALL=OFF -DFLATBUFFERS_BUILD_TESTS=OFF

#编译
make

#安装头文件
#mkdir -p ../include
cp -r ../src/include ../

#安装库
mkdir -p ../lib
mv libflatbuffers* ../lib

#安装可执行文件
mkdir -p ../bin
mv flatc ../bin
mv flathash ../bin

#删除build目录内容
cd -
rm build -rf
rm src -rf

