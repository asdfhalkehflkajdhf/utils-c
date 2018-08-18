#include <string>
#include <set>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <chrono>
#include <sstream>

#include "record.hpp"

#include "data.hpp"

#include <typeinfo>


template <std::size_t opts>
void
yas_serialization_test(size_t iterations)
{
    using namespace yas_test;

    Record r1, r2;

	//写入数字类型值 ，1000个
    for (size_t i = 0; i < kIntegers.size(); i++) {
		//数据结构嵌套测试
        r1.ids.ids.push_back(kIntegers[i]);
    }

	//写入字符串类型值，100个
    for (size_t i = 0; i < kStringsCount; i++) {
        r1.strings.push_back(kStringValue);
    }

    std::string serialized;

	//序列化
    to_string<opts>(r1, serialized);
	//反序列化
    from_string<opts>(r2, serialized);

    if (r1 != r2) {
        throw std::logic_error("yas' case: deserialization failed");
    }

    if (opts & yas::compacted) {
        std::cout << "yas-compact: version = " << YAS_VERSION_STRING << std::endl;
        std::cout << "yas-compact: size = " << serialized.size() << " bytes" << std::endl;
    } else {
        std::cout << "yas: version = " << YAS_VERSION_STRING << std::endl;
        std::cout << "yas: size = " << serialized.size() << " bytes" << std::endl;
    }
	//抽查两个数据是否正常
	// std::cout<<"r1"<<r1.ids.ids[3]<<" r2"<<r2.ids.ids[3]<<std::endl;
	// std::cout<<"r1"<<r1.ids.ids[3]<<" r2"<<r2.ids.ids[3]<<std::endl;
	
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; i++) {
		
		//序列化
        yas::mem_ostream os;
        yas::binary_oarchive<yas::mem_ostream, opts> oa(os);
        oa& r1;
    
		//传输
		auto buf = os.get_intrusive_buffer();

		//反序列化
        yas::mem_istream is(buf);
        yas::binary_iarchive<yas::mem_istream, opts> ia(is);
        ia& r2;
    }
    auto finish = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();

    if (opts & yas::compacted) {
        std::cout << "yas-compact: time = " << duration << " milliseconds" << std::endl << std::endl;
    } else {
        std::cout << "yas: time = " << duration << " milliseconds" << std::endl << std::endl;
    }
}


//版本更新测试
// yas 
	// 1、没有对数据结构进行优化，完全是以字符串的或二进制的方式进行编码。（131992270 bytes）
	
	// 2、使用方便，嵌入在原数据结构中，不需要生成额外库，几行（一般不超过10行）就可以完成序列化，对原始代码的修改和入侵是最少了。
	
	// 3、区分数据初始化和序列化部分，这样在进行数据组合时（初始化）也会有一部分计算时间开销。（先生成数据结构再序列化）


	// 4、不支持版本控制，在表尾是可以的，但不能嵌套。
		// 在根表末尾添加新字段时可以的。


	// 5、反序列化数据可以直接再次进行序列化，不用数据重组
	// 相比同一级别的序列化有， flatbuffers  capnproto 但是后两，对于代码的量有些大。
	// 从支持语言种类上，yas十分少，功能上比较单一。如果是中序列化C++数据足够。
template <std::size_t opts>
void
yas_serialization_version_test(){

    using namespace yas_test;

	//客户端旧版本
    Record r1;
	//服务器新版本，添加字段
	Record r2;
	Record r3;
	//写入数字类型值 ，1000个
    for (size_t i = 0; i < kIntegers.size(); i++) {
		//数据结构嵌套测试
        r1.ids.ids.push_back(kIntegers[i]);
    }

	//写入字符串类型值，100个
    for (size_t i = 0; i < kStringsCount; i++) {
        r1.strings.push_back(kStringValue);
    }

	r1.v=2;
	
	//序列化
	yas::mem_ostream os;
    yas::binary_oarchive<yas::mem_ostream, opts> oa(os);
    oa & r1;
    
    auto buf = os.get_intrusive_buffer();
	//反序列化
	yas::mem_istream is(buf);
	yas::binary_iarchive<yas::mem_istream, opts> ia(is);
	// ia& r2;

	Strings  strings;
	SINT ids;
	SINT2 ids2;
	Strings  add;
	int32_t v;
	// ia & YAS_OBJECT("Record",("v",v));
	// std::cout<<"a"<<v<<std::endl;
	// if(v==1){
			// ia & YAS_OBJECT_NVP("Record",("v",v));
			// ia & YAS_OBJECT("Recor",("1",strings));
			// ia & YAS_OBJECT("Recod",("2",ids));
			// std::cout<<strings.size()<<std::endl;
			// std::cout<<ids.ids[1]<<std::endl;
			// Record r3;
			// ia & r3;
			// std::cout<<r3.strings[1]<<std::endl;
			// std::cout<<r3.ids.ids[1]<<std::endl;

	// }else if (v==2){
			// ia & YAS_OBJECT_NVP("Record",("v",v));
			// ia & strings;
			// ia & ids2;
			// std::cout<<strings.size()<<std::endl;
			// std::cout<<ids2.ids[1]<<std::endl;
	// }
	ia & r2;
	
	// std::count<<add[0]<<std::endl;
	// std::cout<<strings[2]<<std::endl;
	// std::cout<<ids.ids[1]<<std::endl;
	std::cout<<r2.v<<std::endl;
	std::cout<<r2.ids.ids[0]<<std::endl;

	// std::cout<<r2.add[0]<<std::endl;

		//序列化
	yas::mem_ostream os2;
    yas::binary_oarchive<yas::mem_ostream, opts> oa2(os2);
    oa2 & r2;
    
    auto buf2 = os2.get_intrusive_buffer();
	//反序列化
	yas::mem_istream is2(buf2);
	yas::binary_iarchive<yas::mem_istream, opts> ia2(is2);
	// ia& r2;
	ia2 & r3;
	std::cout<<r3.v<<std::endl;
	std::cout<<r3.ids.ids[0]<<std::endl;
	
}


int
main(int argc, char** argv)
{

    if (argc < 2) {
        std::cout << "usage: " << argv[0]
                  << " N [ yas yas-compact yas-version ]";
        std::cout << std::endl << std::endl;
        std::cout << "arguments: " << std::endl;
        std::cout << " N  -- number of iterations" << std::endl << std::endl;
        return EXIT_SUCCESS;
    }

    size_t iterations;

    try {
        iterations = atoi(argv[1]);
    } catch (std::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
        std::cerr << "First positional argument must be an integer." << std::endl;
        return EXIT_FAILURE;
    }

    std::set<std::string> names;

    if (argc > 2) {
        for (int i = 2; i < argc; i++) {
            names.insert(argv[i]);
        }
    }

    std::cout << "performing " << iterations << " iterations" << std::endl << std::endl;

    /*std::cout << "total size: " << sizeof(kIntegerValue) * kIntegersCount + kStringValue.size() * kStringsCount << std::endl;*/

    try {


        if (names.empty() || names.find("yas") != names.end()) {
            // yas_serialization_test<yas::binary | yas::no_header>(iterations);
            yas_serialization_test<yas::binary >(iterations);
        }

        if (names.empty() || names.find("yas-compact") != names.end()) {
            yas_serialization_test<yas::binary | yas::no_header | yas::compacted>(iterations);
        }
		
		if (names.empty() || names.find("yas-version") != names.end()) {
			yas_serialization_version_test<yas::binary | yas::no_header>();
		}

    } catch (std::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
