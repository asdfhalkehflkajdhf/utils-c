 
#ifndef __YAS_INTER_HPP_INCLUDED__
#define __YAS_INTER_HPP_INCLUDED__

#include <vector>
#include <string>

#include <stdint.h>

#include <yas/mem_streams.hpp>
#include <yas/binary_iarchive.hpp>
#include <yas/binary_oarchive.hpp>
#include <yas/std_types.hpp>

namespace yas_inter {

enum NetworkType{NetworkType_a};
enum ADSource{ADSource_a};

enum RequestType{RequestType_a};
enum Platform{Platform_a};
enum ADTemplate{ADTemplate_a};
enum Orientation{Orientation_a};
enum Category{Category_a};
enum InteractiveResourceType{InteractiveResourceType_a};
enum OfferType{OfferType_a};
enum OfferTag{OfferTag_a};

enum ImageSizeEnum{ImageSizeEnum_a};
enum CreativeType{CreativeType_a};


struct SVideo {
    int32_t width;
    int32_t height ;
    int64_t creativeId;
	
	//为了判断两个值是否相等
	bool operator==(const SVideo &other) {
        return (width == other.width 
		&& height == other.height 
		&& creativeId == other.creativeId
		
		);
    }

    bool operator!=(const SVideo &other) {
        return !(*this == other);
    }
	//序列化支持函数

	// template<typename Ar>
	// void serialize(Ar &ar) {
        // ar & YAS_OBJECT("base", x);
	// }
    template<typename Archive>
    void serialize(Archive &ar)
    {
        ar & width 
		& height
		& creativeId;
    }
};




struct CreativeAttr {
    int32_t bitrate;
    int32_t creativeId;
    int64_t subCreativeId;
    int32_t height;
    int32_t width;
    int64_t minOs;
    Orientation orientation; //enum.Orientation ;
    std::string mime; //string ;
    std::string resolution; //string ;
    std::string videoLength; //string ;
    std::string videoResolution; //string ;
    std::string videoSize; //string ;
	
	//为了判断两个值是否相等
	bool operator==(const CreativeAttr &other) {
        return (bitrate == other.bitrate 
		&& creativeId == other.creativeId 
		&& subCreativeId == other.subCreativeId
		&& height == other.height
		&& width == other.width
		&& minOs == other.minOs
		&& orientation == other.orientation

		&& mime == other.mime
		&& resolution == other.resolution
		&& videoLength == other.videoLength
		&& videoResolution == other.videoResolution
		&& videoSize == other.videoSize

		);
    }

    bool operator!=(const CreativeAttr &other) {
        return !(*this == other);
    }

	//序列化支持函数
	//与上边的定义声明次须无关
    template<typename Archive>
    void serialize(Archive &ar)
    {
        ar & bitrate 
		& creativeId
		& subCreativeId
		& height
		& width
		& minOs
		& mime
		& resolution
		& videoLength
		& videoResolution
		& videoSize;

	}
};


struct SCampaignDataFromFrame {
    int64_t campaignId;
    int64_t advertiserId;
    std::string packageName; //string ;
    int32_t countryCnt;
    double price ;
    std::string appName; //string ;
    std::string appDesc; //string ;
    double appScore ;
    int64_t appInstall;
    int32_t direct;
    ADSource adSource;//enum
    int32_t offerIndex;
    Category category;//enum
    int64_t publisherId;
    int64_t netWork;
    std::string netWorkCid; //string ;
    std::set<int64_t> imageSize;//std::set<ImageSizeEnum> imageSize; //set<enum.ImageSizeEnum> ;
    int64_t jumpCnt;
    int64_t detectStatus;
    int64_t detectTimes;
    OfferType offerType ;//enum
    OfferTag offerTag ;//enum
    bool ifRta = false;
    int32_t ifVta = 0;
    int32_t cpx;
    int32_t sqlSubCategory;
    std::vector<int64_t> creativeIdList;
    bool isCampaignCreative  = false;
    std::map<int64_t, std::vector<int64_t>> creativeIdMap; //std::map<CreativeType, std::vector<int64_t>> creativeIdMap;//creativeIdMap: [CreativeIdMap];//map<enum.CreativeType, list<long>> ;
    std::map<int64_t, std::vector<CreativeAttr>> creativeAttrMap;//std::map<CreativeType, std::vector<CreativeAttr>> creativeAttrMap;//map<enum.CreativeType, list<CreativeAttr>> ;
    std::vector<SVideo> videos;
	
	
	//关于判断相等产，可以使用随机抽取测试
	//序列化支持函数
	//与上边的定义声明次须无关
    template<typename Archive>
    void serialize(Archive &ar)
    {
        ar 
		& campaignId
		& advertiserId
		& packageName //string 
		& countryCnt
		& price 
		& appName //string 
		& appDesc //string 
		& appScore 
		& appInstall
		& direct
		& adSource //enum
		& offerIndex
		& category  //enum
		& publisherId
		& netWork
		& netWorkCid //string 
		& imageSize //set<enum.ImageSizeEnum> 
		& jumpCnt
		& detectStatus
		& detectTimes
		& offerType //enum
		& offerTag //enum
		& ifRta 
		& ifVta 
		& cpx
		& sqlSubCategory
		& creativeIdList
		& isCampaignCreative 
		& creativeIdMap//creativeIdMap: [CreativeIdMap]//map<enum.CreativeType, list<long>> 
		& creativeAttrMap//map<enum.CreativeType, list<CreativeAttr>> 
		& videos;
	}
	
};


struct SSmartPV {
    std::string packageName; //string ;
    std::string version; //string ;
	
	//关于判断相等产，可以使用随机抽取测试
	//序列化支持函数
	//与上边的定义声明次须无关
    template<typename Archive>
    void serialize(Archive &ar)
    {
        ar & packageName 
		& version;
    }
};


struct SRankerInput {
    std::string cityCode;
    std::string countryCode; //string ;
    std::string adType; //string ;
    std::string devId; //string ;
    NetworkType netWorkType; //enum
    int32_t adNum;
    int64_t timestamp;
    std::string timeZone; //string ;
    int64_t unitId;
    int64_t appId;
    std::string appName; //string ;
    std::string language; //string ;
    int64_t publisherId;
    std::vector<int64_t> sourceList; //std::vector<ADSource> sourceList;//enum
    std::string scenario; //string ;
    RequestType requestType;//enum.
    Platform platform ;//enum.
    std::string ip; //string ;
    std::string unitSize; //string ;
    std::string screenSize; //string ;
    ADTemplate templateType;//enum.
    std::string sessionId; //string ;
    std::string deviceModel; //string ;
    Orientation orientation;//enum.
    Orientation unitOrientation;//enum.
    std::vector<int64_t> fixedOrderVect;
    std::vector<SCampaignDataFromFrame> campaignData;
    std::vector<SCampaignDataFromFrame> campaignDataPreclick;
    std::vector<int64_t> showedCampaignId ;
    std::string imei; //string ;
    std::string gaid; //string ;
    std::string idfa; //string ;
    bool preclick = false;
    int64_t trueNum;
    std::vector<int64_t> preclickCampaignIds;
    std::string deviceBrand; //string ;
    std::vector<int64_t> fixedOrderRba;
    std::map<int32_t, std::set<int64_t> > fixedOrderNormal;//map<int, set<long> > ;
    Category category;//enum.
    int64_t offset;
    std::string versionName; //string ;
    std::vector<SSmartPV> smartPvList;
    std::vector<SSmartPV> powerPvList;
    std::vector<SSmartPV> cleverPvList;
    bool ifIncludePackagenameSet  = false;
    std::string osVersion; //string ;
    std::string sdkVersion; //string ;
    std::string mnc; //string ;
    std::string mcc; //string ;
    std::string dmpCategory; //string ;
    std::string dmpSubCategory; //string ;
    std::string appIdCategory; //string ;
    std::string appIdSubCategory; //string ;
    int32_t adTypeInt;
    std::vector<int64_t> excludeIdList;
    std::vector<int64_t> installIdList;
    std::vector<int64_t> prePid;
    std::string rankerInfo; //string ;
    bool isNativeVideo = false;
    std::map<int64_t, double> tryNewList; //map<long, double> ;
    double tryNewPercent ;
    bool ifLowEfficientUnitRequest  = false;
    std::string ua; //string ;
    std::string rankerName; //string ;
    int32_t publisherType;
    std::string requestId; //string ;
    bool debugMode;
    std::vector<InteractiveResourceType> resourceTypeList ;//[enum]
	
	
	//关于判断相等产，可以使用随机抽取测试
	//序列化支持函数
	//与上边的定义声明次须无关
    template<typename Archive>
    void serialize(Archive &ar)
    {
        ar
		& cityCode
		& countryCode //string 
		& adType //string 
		& devId //string 
		& netWorkType //enum
		& adNum
		& timestamp
		& timeZone //string 
		& unitId
		& appId
		& appName //string 
		& language //string 
		& publisherId
		& sourceList//enum
		& scenario //string 
		& requestType//enum.
		& platform //enum.
		& ip //string 
		& unitSize //string 
		& screenSize //string 
		& templateType//enum.
		& sessionId //string 
		& deviceModel //string 
		& orientation//enum.
		& unitOrientation//enum.
		& fixedOrderVect
		& campaignData
		& campaignDataPreclick
		& showedCampaignId 
		& imei //string 
		& gaid //string 
		& idfa //string 
		& preclick 
		& trueNum
		& preclickCampaignIds
		& deviceBrand //string 
		& fixedOrderRba
		& fixedOrderNormal//map<int, set<long> > 
		& category//enum.
		& offset
		& versionName //string 
		& smartPvList
		& powerPvList
		& cleverPvList
		& ifIncludePackagenameSet
		& osVersion //string
		& sdkVersion //string 
		& mnc //string 
		& mcc //string 
		& dmpCategory //string 
		& dmpSubCategory //string 
		& appIdCategory //string 
		& appIdSubCategory //string 
		& adTypeInt
		& excludeIdList
		& installIdList
		& prePid
		& rankerInfo //string 
		& isNativeVideo
		& tryNewList //map<long, double> 
		& tryNewPercent 
		& ifLowEfficientUnitRequest
		& ua //string 
		& rankerName //string 
		& publisherType
		& requestId //string 
		& debugMode
		& resourceTypeList ;//[enum]
    }
};



struct SRankedCampaignInfo {
    int64_t campaignId;
    ADSource adSource; // enum.ADSource ;
    bool preclick = false;
    OfferType offerType ; //enum.OfferType ;
    bool force_to_vba = false;
    bool if_download_detect_try_b2t = false;
	std::map<int64_t, int64_t> creativeIdMap;//std::map<CreativeType, int64_t> creativeIdMap;//   creativeIdMap: [CreativeIdMap_2]; //map<enum.CreativeType, i64> ;
    std::map<int64_t, std::string> dco;//std::map<CreativeType, std::string> dco; //map<enum.CreativeType, string> ; 
	
		
	//关于判断相等产，可以使用随机抽取测试
	//序列化支持函数
	//与上边的定义声明次须无关
    template<typename Archive>
    void serialize(Archive &ar)
    {
        ar
		& campaignId
		& adSource // enum.ADSource ;
		&  preclick
		&  offerType  //enum.OfferType ;
		&  force_to_vba
		&  if_download_detect_try_b2t
		&  creativeIdMap //   creativeIdMap: [CreativeIdMap_2]; //map<enum.CreativeType, i64> ;
		&  dco; //map<enum.CreativeType, string> ; 
	}
};

struct SRankerOutput {
    std::vector<SRankedCampaignInfo> campaignList ;
    std::vector<std::string> accessLogFields;
    std::string algoFeatJson; //string ;
    bool ifLowerImp = false;
    std::string strategy; //string ;
    std::string debugInfo; //string ;
    int32_t status ;
    InteractiveResourceType resourceType; //enum.InteractiveResourceType  = enum.InteractiveResourceType.UNKNOWN;
	
	//关于判断相等产，可以使用随机抽取测试
	//序列化支持函数
	//与上边的定义声明次须无关
    template<typename Archive>
    void serialize(Archive &ar)
    {
        ar
		& campaignList
		& accessLogFields
		& algoFeatJson //string ;
		& ifLowerImp
		& strategy //string ;
		& debugInfo //string ;
		& status
		& resourceType; //enum.InteractiveResourceType  = enum.InteractiveResourceType.UNKNOWN;
	}
};

enum SErrorCode {
    kSucc = 0,
    kFail = 1,
};

struct SFailureException {
    SErrorCode errorCode ;
    std::string message; //string ;
    std::string requestId; //string ;
	
	//关于判断相等产，可以使用随机抽取测试
	//序列化支持函数
	//与上边的定义声明次须无关
    template<typename Archive>
    void serialize(Archive &ar)
    {
        ar
		& errorCode
		& message
		& requestId; //string ;
	}
};


struct SDictMeta {
    std::string  dict_name; //string ;
    std::string  dict_path; //string ;
    int32_t cache_size ;
	
	
	//关于判断相等产，可以使用随机抽取测试
	//序列化支持函数
	//与上边的定义声明次须无关
    template<typename Archive>
    void serialize(Archive &ar)
    {
        ar
		& dict_name
		& dict_path
		& cache_size; //string ;
	}
};

struct RankerSrv {
    SRankerOutput Rank;
	SRankerInput rankerInput ;
	SFailureException qe;
	SDictMeta param ;
	
	//关于判断相等产，可以使用随机抽取测试
	//序列化支持函数
	//与上边的定义声明次须无关
    template<typename Archive>
    void serialize(Archive &ar)
    {
        ar
		& Rank
		& rankerInput
		& qe
		& param; //string ;
	}
};

template<std::size_t opts>
void to_string(const RankerSrv &record, std::string &data) {
    yas::mem_ostream os;
    yas::binary_oarchive<yas::mem_ostream, opts> oa(os);
    oa & record;
    
    auto buf = os.get_intrusive_buffer();
    data.assign(buf.data, buf.size);
}

template<std::size_t opts>
void from_string(RankerSrv &record, const std::string &data) {
    yas::mem_istream is(data.c_str(), data.size());
    yas::binary_iarchive<yas::mem_istream, opts> ia(is);
    ia & record;
}



} // namespace

#endif
