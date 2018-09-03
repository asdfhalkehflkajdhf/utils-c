#include <iostream>
#include <typeinfo>
#include <tuple>        // std::tuple, std::get, std::tie, std::ignore
#include <map>
#include <vector>
#include <memory>
#include <sstream>	//使用stringstream需要引入这个头文件

using namespace std;

#define TYPEID(v) (typeid(v).name())
class oneparse{
	enum local{
		ARGP_VALUE=0,
		ARGP_TYPE=1,
		ARGP_DEST=2,
		ARGP_GROUP=3,
		ARGP_DEF=4,
		ARGP_SHORT=5,
		ARGP_LONG=6,
		ARGP_MAX
	};
	//值, (类型，描述，组，默认值) 短，长
	vector<string> _vs_list;
public:
	oneparse(const std::string short_d, const std::string long_d, 
		const std::string type_d, const std::string default_v="",

		const std::string dest="",  const std::string group="root")
		: _vs_list({default_v, type_d, dest, group, default_v, short_d, long_d})
	{
		// _vs_list=(default_v, type_d, dest, group, default_v, short_d, long_d);
	}
	
	auto get_value(){
		// cout<<__FUNCTION__<<" "<<_vs_list[ARGP_VALUE]<<endl;
		return _vs_list[ARGP_VALUE];
	}
	
	// std::tie(myint, std::ignore, mychar) = mytuple;
	void set_value(const string &v){
		// cout<<__FUNCTION__<<" "<<_vs_list[ARGP_VALUE]<<endl;
		_vs_list[ARGP_VALUE]=v;
	}
	
	auto get_type(){
		// cout<<__FUNCTION__<<" "<<_vs_list[ARGP_TYPE]<<endl;
		return _vs_list[ARGP_TYPE];
	}

	auto get_dest(){
		// cout<<__FUNCTION__<<" "<<_vs_list[ARGP_DEST]<<endl;
		return _vs_list[ARGP_DEST];
	}

	auto get_group(){
		// cout<<__FUNCTION__<<" "<<_vs_list[ARGP_GROUP]<<endl;
		return _vs_list[ARGP_GROUP];
	}

	auto get_def(){
		// cout<<__FUNCTION__<<" "<<_vs_list[ARGP_DEF]<<endl;
		return _vs_list[ARGP_DEF];
	}
	
	auto get_short_name(){
		// cout<<__FUNCTION__<<" "<<_vs_list[ARGP_SHORT]<<endl;
		return _vs_list[ARGP_SHORT];
	}
	
	auto get_long_name(){
		// cout<<__FUNCTION__<<" "<<_vs_list[ARGP_LONG]<<endl;
		return _vs_list[ARGP_LONG];
	}
};
using oneparse_ptr = std::shared_ptr<oneparse>;

class argparse{
	using key_list = vector<string>;
	
	int index=0;
	string exec_name="";
	
	//使用多级映射，是因为可能一个参数对应多个形式
	// map<string, int> args_to_id; //用于用户输入参数查找方便
	// map<string, int> argu_to_id; //用于程序查找参数方便
	map<int, key_list> id_to_argu; //输出help查找方便
	map<string, oneparse_ptr> argu_map; //保存每个参数的值

	template <typename T>
	T stringToAuto(const string& str, const string &type_str){
		istringstream iss(str);
		T res;
		if(TYPEID(T) == TYPEID(bool)){
			res = (str[0] == 't' || str[0]=='T')?true:false;
		}else if(TYPEID(T) == type_str){
			iss >> res;
		}else{
			cout<<"parse:type is error!"<<endl;
			show_help();
			exit(1);
		}
		return res;
	}
public:
	argparse(){
		add_argument("h", "help", TYPEID(bool));
	};

	void add_argument(const std::string short_d, const std::string long_d, 
		const std::string type_d, const std::string default_v="", 
		
		const std::string dest="", const std::string group="root"){
		//保存的是没有-- -的参数
		oneparse_ptr top(new oneparse( short_d, long_d, type_d, default_v, dest, group));
		
		id_to_argu[index]={long_d, short_d};
		argu_map[long_d]=top;
		argu_map[short_d]=top;
		argu_map["--"+long_d]=top;
		argu_map["-"+short_d]=top;
		
		// cout<<__FUNCTION__<<" | "<<
			// argu_map[index]->get_value()<<" | "<<
			// argu_map[index]->get_type()<<" | "<<
			// argu_map[index]->get_dest()<<" | "<<
			// argu_map[index]->get_def()<<endl;
		
		++index;
	};
	
	void add_argument(oneparse_ptr &top){
		std::string short_d = top->get_short_name();
		std::string long_d = top->get_long_name();
		
		id_to_argu[index]={"--"+long_d, "-"+short_d};
		
		argu_map[long_d]=top;
		argu_map[short_d]=top;
		argu_map["--"+long_d]=top;
		argu_map["-"+short_d]=top;

		++index;		
	}
	
	void show_help(){
		cout<<"usage: "<<exec_name<<endl;
		//id为0则最后显示，因为是help固定值
		cout<<"\tpositional arguments:"<<endl;
		for(int i=1;i<index; ++i){
			string opt=id_to_argu[i][0]+", "+id_to_argu[i][1];
			oneparse_ptr &top = argu_map[id_to_argu[i][0]];
			
			cout<<"\t\t"<<opt<<"\t"<<top->get_dest()<<
				"\t default["<<top->get_def()<<"]"<<
				"\t type["<<top->get_type()<<"]"<<endl;
		}
		
		//显示help 参数
		cout<<endl;
		string opt=id_to_argu[0][0]+", "+id_to_argu[0][1];
		oneparse_ptr &top = argu_map[id_to_argu[0][0]];
		cout<<"\t\t"<<opt<<"\t"<<top->get_dest()<<
			"\t default["<<top->get_def()<<"]"<<
			"\t type["<<top->get_type()<<"]"<<endl;
		
						
	};
	
	void analyze(int argc, char **args){
		exec_name=args[0];
		int id;

		for(int i=1;i<argc; ++i){
			//参数没有发现，
			if(argu_map.find(args[i])==argu_map.end()){
				cout<<"parse:["<<args[i]<<"] is not find! "<<endl;
				show_help();
				exit(1);
			}
			
			if(string(args[i])=="-h" || string(args[i])=="--help"){
				show_help();
				exit(1);
			}
			
			oneparse_ptr &top = argu_map[args[i]];

			//查看类型,因为bool类型只是以是否出现，并没有后续参数。需要程序添加
			if(top->get_type() == TYPEID(bool)){
				top->set_value("t");
			}else{
				top->set_value(args[i++]);
			}
		}
	};

	template <typename T >
	T get(std::string k){
		//获取参数时，失败
		if(argu_map.find(k)==argu_map.end()){
			cout<<"parse:["<<k<<"] is not find! "<<endl;
			show_help();
			exit(1);
		}
		//查看类型
		oneparse_ptr &top = argu_map[k];
		
		istringstream iss(top->get_value());
		T res;
		if(TYPEID(T) == TYPEID(bool)){
			res = (k[0] == 't' || k[0]=='T')?true:false;
		}else if(TYPEID(T) == top->get_type()){
			iss >> res;
		}else{
			cout<<"parse:["<<k<<"] type["<<TYPEID(T)<<"] is error!"<<endl;
			show_help();
			exit(1);
		}

		return res;
	};
};





