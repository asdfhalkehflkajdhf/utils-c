
#ifndef __YAS_RECORD_HPP_INCLUDED__
#define __YAS_RECORD_HPP_INCLUDED__

#include <vector>
#include <string>

#include <stdint.h>

#include <yas/mem_streams.hpp>
#include <yas/binary_iarchive.hpp>
#include <yas/binary_oarchive.hpp>
#include <yas/std_types.hpp>

namespace yas_test {

typedef std::vector<int64_t>     Integers;
typedef std::vector<std::string> Strings;

struct SINT{
		int32_t v;

	Integers ids;
    bool operator==(const SINT &other) {
        return (ids == other.ids );
    }

    bool operator!=(const SINT &other) {
        return !(*this == other);
    }

    template<typename Archive>
    void serialize(Archive &ar)
    {
        ar & v & ids;
    }

};



struct Record {

	int32_t v;
	struct SINT ids;
    // Integers ids;
    Strings  strings;

    bool operator==(const Record &other) {
        return (ids == other.ids && strings == other.strings);
    }

    bool operator!=(const Record &other) {
        return !(*this == other);
    }

    template<typename Archive>
    void serialize(Archive &ar)
    {
		//没有版本控制，必须按顺序来读取
        ar & v & strings & ids;
		// 位置相关的，不能随意换位置
		// ar & YAS_OBJECT("Record"
			// , ("v", v)
			// , ("1", strings)
			// , ("2", ids)
		// );
    }
};

struct SINT2{
	int32_t v;
	SINT2(int32_t tv):v(tv){};
	SINT2(){};
	Integers ids;
	int32_t n;
    bool operator==(const SINT &other) {
        return (ids == other.ids );
    }

    bool operator!=(const SINT &other) {
        return !(*this == other);
    }

    template<typename Archive>
    void serialize(Archive &ar)
    {
		if(v==2)
			ar & v & ids & n ;
		if(v==1)
			ar & v & ids;
    }

};

struct Record2 {
	int32_t v;
	struct SINT2 ids;
    // Integers ids;
    Strings  strings;
    Strings  add={"test1","test2"};

	Record2(int32_t _v):v(_v){};
	Record2(){};


    template<typename Archive>
    void serialize(Archive &ar)
    {
		if(v==2)
        ar & v & strings & ids & add;
		if(v==1)
        ar & v & strings & ids ;
    }
};

template<std::size_t opts>
void to_string(const Record &record, std::string &data) {
    yas::mem_ostream os;
    yas::binary_oarchive<yas::mem_ostream, opts> oa(os);
    oa & record;
    
    auto buf = os.get_intrusive_buffer();
    data.assign(buf.data, buf.size);
}

template<std::size_t opts>
void from_string(Record &record, const std::string &data) {
    yas::mem_istream is(data.c_str(), data.size());
    yas::binary_iarchive<yas::mem_istream, opts> ia(is);
    ia & record;
}

} // namespace

#endif
