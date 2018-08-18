#include <string>
#include <set>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <chrono>
#include <sstream>

#include "inter.hpp"
#include "data.hpp"
#include <typeinfo>

std::string getStringValue(){
	static int i=0;
	return kStringValue+std::to_string(i++);
	
}

template <std::size_t opts>
void
yas_serialization_test(size_t iterations)
{
	std::cout<<"yas_serialization_test"<<std::endl;
    using namespace yas_inter;
	int showk=0;

		std::string serialized;
		RankerSrv r1, r2;

		std::vector<int64_t> fov_int64_50={			1,2,3,4,5,6,7,8,9,10,
				11,12,13,14,15,16,17,18,19,20,
				21,22,23,24,25,26,27,28,29,30,
				31,32,33,34,35,36,37,38,39,40,
				41,42,43,44,45,46,47,48,49,50};
		
		
		//添加数据
		r1.rankerInput.cityCode=getStringValue();
		r1.rankerInput.countryCode=getStringValue();
		r1.rankerInput.adType=getStringValue();
		r1.rankerInput.devId=getStringValue();
		r1.rankerInput.timeZone=getStringValue();
		r1.rankerInput.appName=getStringValue();
		r1.rankerInput.language=getStringValue();
		for(int i=0;i<50;++i)
			r1.rankerInput.sourceList.push_back(ADSource_a);
		r1.rankerInput.scenario = getStringValue();
		r1.rankerInput.ip=getStringValue();
		r1.rankerInput.unitSize=getStringValue();
		r1.rankerInput.screenSize=getStringValue();
		r1.rankerInput.sessionId=getStringValue();
		r1.rankerInput.deviceModel=getStringValue();
		for(int i=0;i<50;++i){
			r1.rankerInput.fixedOrderVect.push_back(i);
		}
			SCampaignDataFromFrame scdff;
			scdff.packageName=getStringValue();
			scdff.appName=getStringValue();
			scdff.appDesc=getStringValue();
			scdff.netWorkCid=getStringValue();
			for(int i =0;i<50;++i)
				scdff.imageSize.insert(i);
			for(int i =0;i<50;++i)
				scdff.creativeIdList.push_back(i);
			for(int i =0;i<50;++i){
				std::vector<int64_t> t(50,1);
				scdff.creativeIdMap[i]=t;
			}
			for(int i =0;i<50;++i){
				CreativeAttr t;
				t.mime=getStringValue();
				t.resolution=getStringValue();
				t.videoLength=getStringValue();
				t.videoResolution=getStringValue();
				t.videoSize=getStringValue();
				std::vector<CreativeAttr> tv(50,t);
				scdff.creativeAttrMap[i]=tv;
			}

			for(int i =0;i<50;++i){
				SVideo t;
				scdff.videos.push_back(t);
			}
		for(int i=0;i<50;++i)
			r1.rankerInput.campaignData.push_back(scdff);
		for(int i=0;i<50;++i)
			r1.rankerInput.campaignDataPreclick.push_back(scdff);
		for(int i=0;i<50;++i)
			r1.rankerInput.showedCampaignId.push_back(i);
		r1.rankerInput.imei=getStringValue();
		r1.rankerInput.gaid=getStringValue();
		r1.rankerInput.idfa=getStringValue();
		for(int i=0;i<50;++i)
			r1.rankerInput.preclickCampaignIds.push_back(i);
		r1.rankerInput.deviceBrand=getStringValue();
		for(int i=0;i<50;++i)
			r1.rankerInput.fixedOrderRba.push_back(i);
		for(int i=0;i<50;++i){
			
			r1.rankerInput.fixedOrderNormal[i]=std::set<int64_t>(fov_int64_50.begin(), fov_int64_50.end()
				
			);
		}
		
		r1.rankerInput.versionName=getStringValue();
			SSmartPV sspv;
			sspv.packageName=getStringValue();
			sspv.version=getStringValue();
		for(int i=0;i<50;++i)
			r1.rankerInput.smartPvList.push_back(sspv);
		for(int i=0;i<50;++i)
			r1.rankerInput.powerPvList.push_back(sspv);
		for(int i=0;i<50;++i)
			r1.rankerInput.cleverPvList.push_back(sspv);
		
		r1.rankerInput.osVersion=getStringValue();
		r1.rankerInput.sdkVersion=getStringValue();
		r1.rankerInput.mnc=getStringValue();
		r1.rankerInput.mcc=getStringValue();
		r1.rankerInput.dmpCategory=getStringValue();
		r1.rankerInput.dmpSubCategory=getStringValue();
		r1.rankerInput.appIdCategory=getStringValue();
		r1.rankerInput.appIdSubCategory=getStringValue();
		
		r1.rankerInput.excludeIdList.insert(r1.rankerInput.excludeIdList.begin(), 50, 6);
		r1.rankerInput.installIdList.insert(r1.rankerInput.installIdList.begin(), 50, 6);
		r1.rankerInput.prePid.insert(r1.rankerInput.prePid.begin(), 50, 6);
		r1.rankerInput.rankerInfo=getStringValue();
		
		for(int i=0;i<50;++i)
			r1.rankerInput.tryNewList[i]=i*1.0;
		r1.rankerInput.ua=getStringValue();
		r1.rankerInput.rankerName=getStringValue();
		r1.rankerInput.requestId=getStringValue();
		
		r1.rankerInput.resourceTypeList.insert(r1.rankerInput.resourceTypeList.begin(), 50, InteractiveResourceType_a);
		//
			SRankedCampaignInfo srci;
			for(int i=0;i<50;++i)
				srci.creativeIdMap[i]=i;
			for(int i=0;i<50;++i)
				srci.dco[i]=getStringValue();
		for(int i=0;i<50;++i)
			r1.Rank.campaignList.push_back(srci);
		for(int i=0;i<50;++i)
			r1.Rank.accessLogFields.push_back(getStringValue());
		r1.Rank.strategy = getStringValue();
		r1.Rank.debugInfo = getStringValue();
		
		//
		r1.qe.message = getStringValue();
		r1.qe.requestId = getStringValue();
		
		//
		r1.param.dict_name = getStringValue();
		r1.param.dict_path = getStringValue();
	//进行迭代
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; i++) {

		//序列化
		// to_string<opts>(r1, serialized);
		//序列化
        yas::mem_ostream os;
        yas::binary_oarchive<yas::mem_ostream, opts> oa(os);
        oa& r1;		
		//传输
				//传输
		auto buf = os.get_intrusive_buffer();
		// std::cout<<typeid(buf).name()<<std::endl;
		// std::cout<<buf.data<<std::endl;
		// std::cout<<buf.size<<std::endl;
		// buf.data buf.size


		if(showk++==0 ){
			if ( opts & yas::compacted) {
				std::cout << "yas-compact: version = " << YAS_VERSION_STRING << std::endl;
				std::cout << "yas-compact: size = " << buf.size << " bytes" << std::endl;
			} else {
				std::cout << "yas: version = " << YAS_VERSION_STRING << std::endl;
				std::cout << "yas: size = " << buf.size << " bytes" << std::endl;
			}
		}


    
		//反序列化
		// from_string<opts>(r2, serialized);

		// 随机抽样检测
		{
			// std::cout<<r2.rankerInput.countryCode<<std::endl;
			// std::cout<<r2.rankerInput.campaignData[3].packageName<<std::endl;
			// std::cout<<r2.rankerInput.campaignDataPreclick[3].packageName<<std::endl;
		}
		// if (r1 != r2) {
			// throw std::logic_error("yas' case: deserialization failed");
		// }
		
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


template <std::size_t opts>
void
yas_init_and_serialization_test(size_t iterations)
{
	std::cout<<"yas_init_and_serialization_test"<<std::endl;
    using namespace yas_inter;
	int showk=0;
	//进行迭代
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; i++) {

		std::string serialized;
		RankerSrv r1, r2;

		std::vector<int64_t> fov_int64_50={			1,2,3,4,5,6,7,8,9,10,
				11,12,13,14,15,16,17,18,19,20,
				21,22,23,24,25,26,27,28,29,30,
				31,32,33,34,35,36,37,38,39,40,
				41,42,43,44,45,46,47,48,49,50};
		
		
		//添加数据
		r1.rankerInput.cityCode=getStringValue();
		r1.rankerInput.countryCode=getStringValue();
		r1.rankerInput.adType=getStringValue();
		r1.rankerInput.devId=getStringValue();
		r1.rankerInput.timeZone=getStringValue();
		r1.rankerInput.appName=getStringValue();
		r1.rankerInput.language=getStringValue();
		for(int i=0;i<50;++i)
			r1.rankerInput.sourceList.push_back(ADSource_a);
		r1.rankerInput.scenario = getStringValue();
		r1.rankerInput.ip=getStringValue();
		r1.rankerInput.unitSize=getStringValue();
		r1.rankerInput.screenSize=getStringValue();
		r1.rankerInput.sessionId=getStringValue();
		r1.rankerInput.deviceModel=getStringValue();
		for(int i=0;i<50;++i){
			r1.rankerInput.fixedOrderVect.push_back(i);
		}
			SCampaignDataFromFrame scdff;
			scdff.packageName=getStringValue();
			scdff.appName=getStringValue();
			scdff.appDesc=getStringValue();
			scdff.netWorkCid=getStringValue();
			for(int i =0;i<50;++i)
				scdff.imageSize.insert(i);
			for(int i =0;i<50;++i)
				scdff.creativeIdList.push_back(i);
			for(int i =0;i<50;++i){
				std::vector<int64_t> t(50,1);
				scdff.creativeIdMap[i]=t;
			}
			for(int i =0;i<50;++i){
				CreativeAttr t;
				t.mime=getStringValue();
				t.resolution=getStringValue();
				t.videoLength=getStringValue();
				t.videoResolution=getStringValue();
				t.videoSize=getStringValue();
				std::vector<CreativeAttr> tv(50,t);
				scdff.creativeAttrMap[i]=tv;
			}

			for(int i =0;i<50;++i){
				SVideo t;
				scdff.videos.push_back(t);
			}
		for(int i=0;i<50;++i)
			r1.rankerInput.campaignData.push_back(scdff);
		for(int i=0;i<50;++i)
			r1.rankerInput.campaignDataPreclick.push_back(scdff);
		for(int i=0;i<50;++i)
			r1.rankerInput.showedCampaignId.push_back(i);
		r1.rankerInput.imei=getStringValue();
		r1.rankerInput.gaid=getStringValue();
		r1.rankerInput.idfa=getStringValue();
		for(int i=0;i<50;++i)
			r1.rankerInput.preclickCampaignIds.push_back(i);
		r1.rankerInput.deviceBrand=getStringValue();
		for(int i=0;i<50;++i)
			r1.rankerInput.fixedOrderRba.push_back(i);
		for(int i=0;i<50;++i){
			
			r1.rankerInput.fixedOrderNormal[i]=std::set<int64_t>(fov_int64_50.begin(), fov_int64_50.end()
				
			);
		}
		
		r1.rankerInput.versionName=getStringValue();
			SSmartPV sspv;
			sspv.packageName=getStringValue();
			sspv.version=getStringValue();
		for(int i=0;i<50;++i)
			r1.rankerInput.smartPvList.push_back(sspv);
		for(int i=0;i<50;++i)
			r1.rankerInput.powerPvList.push_back(sspv);
		for(int i=0;i<50;++i)
			r1.rankerInput.cleverPvList.push_back(sspv);
		
		r1.rankerInput.osVersion=getStringValue();
		r1.rankerInput.sdkVersion=getStringValue();
		r1.rankerInput.mnc=getStringValue();
		r1.rankerInput.mcc=getStringValue();
		r1.rankerInput.dmpCategory=getStringValue();
		r1.rankerInput.dmpSubCategory=getStringValue();
		r1.rankerInput.appIdCategory=getStringValue();
		r1.rankerInput.appIdSubCategory=getStringValue();
		
		r1.rankerInput.excludeIdList.insert(r1.rankerInput.excludeIdList.begin(), 50, 6);
		r1.rankerInput.installIdList.insert(r1.rankerInput.installIdList.begin(), 50, 6);
		r1.rankerInput.prePid.insert(r1.rankerInput.prePid.begin(), 50, 6);
		r1.rankerInput.rankerInfo=getStringValue();
		
		for(int i=0;i<50;++i)
			r1.rankerInput.tryNewList[i]=i*1.0;
		r1.rankerInput.ua=getStringValue();
		r1.rankerInput.rankerName=getStringValue();
		r1.rankerInput.requestId=getStringValue();
		
		r1.rankerInput.resourceTypeList.insert(r1.rankerInput.resourceTypeList.begin(), 50, InteractiveResourceType_a);
		//
			SRankedCampaignInfo srci;
			for(int i=0;i<50;++i)
				srci.creativeIdMap[i]=i;
			for(int i=0;i<50;++i)
				srci.dco[i]=getStringValue();
		for(int i=0;i<50;++i)
			r1.Rank.campaignList.push_back(srci);
		for(int i=0;i<50;++i)
			r1.Rank.accessLogFields.push_back(getStringValue());
		r1.Rank.strategy = getStringValue();
		r1.Rank.debugInfo = getStringValue();
		
		//
		r1.qe.message = getStringValue();
		r1.qe.requestId = getStringValue();
		
		//
		r1.param.dict_name = getStringValue();
		r1.param.dict_path = getStringValue();

		//序列化
		// to_string<opts>(r1, serialized);
		//序列化
        yas::mem_ostream os;
        yas::binary_oarchive<yas::mem_ostream, opts> oa(os);
        oa& r1;		
		//传输
				//传输
		auto buf = os.get_intrusive_buffer();
		// std::cout<<typeid(buf).name()<<std::endl;
		// std::cout<<buf.data<<std::endl;
		// std::cout<<buf.size<<std::endl;
		// buf.data buf.size


		if(showk++==0 ){
			if ( opts & yas::compacted) {
				std::cout << "yas-compact: version = " << YAS_VERSION_STRING << std::endl;
				std::cout << "yas-compact: size = " << buf.size << " bytes" << std::endl;
			} else {
				std::cout << "yas: version = " << YAS_VERSION_STRING << std::endl;
				std::cout << "yas: size = " << buf.size << " bytes" << std::endl;
			}
		}


    
		//反序列化
		// from_string<opts>(r2, serialized);

		// 随机抽样检测
		{
			// std::cout<<r2.rankerInput.countryCode<<std::endl;
			// std::cout<<r2.rankerInput.campaignData[3].packageName<<std::endl;
			// std::cout<<r2.rankerInput.campaignDataPreclick[3].packageName<<std::endl;
		}
		// if (r1 != r2) {
			// throw std::logic_error("yas' case: deserialization failed");
		// }
		
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


int
main(int argc, char** argv)
{

    if (argc < 2) {
        std::cout << "usage: " << argv[0]
                  << " N [ yas yas-init-seri]";
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

    /*std::cout << "total size: " << sizeof(kIntegerValue) * kIntegersCount + getStringValue().size() * kStringsCount << std::endl;*/

    try {


        if (names.empty() || names.find("yas") != names.end()) {
            yas_serialization_test<yas::binary | yas::no_header>(iterations);
        }

        if (names.empty() || names.find("yas-init-seri") != names.end()) {
			// 运行会出错。不知道那里没有写对，在数据转换时
            yas_init_and_serialization_test<yas::binary | yas::no_header>(iterations);
        }
    } catch (std::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
