github地址： https://github.com/niXman/yas

序列化库对比 https://github.com/thekvs/cpp-serializers
yum -y install autoconf automake libtool flex bison pkgconfig gcc-c++ boost-devel libevent-devel zlib-devel Python-devel ruby-devel crypto-utils openssl openssl-devel


－、capnp 

1、档十分少，目录只是开源社区在讨论。 https://capnproto.org/cxxrpc.html
2、由个人进行维护。
3、并没有发现有大的公司在使用。

4、依赖kj,和一些unix 特有api。
	
	yum install autoconf automake

	编译安装
	git clone https://github.com/sandstorm-io/capnproto.git
	cd capnproto/c++
	autoreconf -i
	./configure
	make -j6 check
	sudo make install

	
	wget https://capnproto.org/capnproto-c++-0.6.1.tar.gz
	tar zxf capnproto-c++-0.6.1.tar.gz
	cd capnproto-c++-0.6.1
	./configure
	make -j6 check
	sudo make install
	
	生成可执行文件和库所在目录
	.libs
	
	
	
	
flatbuffers
1、文档完善，https://google.github.io/flatbuffers/
2、由google进行维护，

3、在谷歌内部和feacbook在使用。
4、在游戏应用上比较多。

5、无任何依赖。

6、相比capnp ，在后期扩展，升级，维护上，方便。

	编译安装
	git clone https://github.com/google/flatbuffers.git
	该发行版附带一个cmake文件，允许您为任何平台构建项目/ make文件。有关详细信息cmake，请参阅https://www.cmake.org。简而言之，根据您的平台，使用以下方法之一：

	cmake -G"Unix Makefiles"
	cmake -G"Visual Studio 10"
	cmake -G"Xcode"
	然后，为您的平台正常构建。这应该导致flatc可执行文件，对于后续步骤至关重要。请注意，要使用clang而不是gcc，您可能需要设置环境变量，例如CC=/usr/bin/clang CXX=/usr/bin/clang++ cmake -G "Unix Makefiles"。

	make
	make install

	
	
在时间上，YAS更快。
1、文档更少	，没有找到官方网站。https://github.com/niXman/yas
2、支持以下类型的存档：
二进制
文本
json（不完全遵守）

3、由个人进行维护。
4、有一些公司在使用
5、无任何依赖。


	安装
	wget http://sourceforge.net/projects/boost/files/boost/1.64.0/boost_1_64_0.tar.bz2
    tar -jxf boost_1_64_0.tar.bz2
    cp -r boost_1_64_0/boost /usr/local/include
	
	git clone https://github.com/niXman/yas.git
	before_script:
	  - cd $TRAVIS_BUILD_DIR/tests/base
	  - mkdir build
	  - cd build
	  - cmake -DCMAKE_BUILD_TYPE=Release ..

	script:
	  - cd $TRAVIS_BUILD_DIR/tests/base/build
	  - cmake --build . --config Release
	  - ./yas-base-test binary && ./yas-base-test binary compacted && ./yas-base-test json && ./yas-base-test json compacted && ./yas-base-test text
	  
	  
	  
cpp-serializers
	  https://github.com/thekvs/cpp-serializers.git
	  
	  
	  $ git clone https://github.com/thekvs/cpp-serializers.git
	$ mkdir /path/to/build-root/
	$ cd /path/to/build-root/
	$ cmake ../cpp-serializers -DCMAKE_BUILD_TYPE=Release
	$ make
