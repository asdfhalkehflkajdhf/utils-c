说明：

	argparse argp;
	
	//添加参数选项
			void add_argument(const std::string short_d, const std::string long_d, 
		const std::string type_d, const std::string default_v="", 
		const std::string dest="", const std::string group="root")

		短参数名，长参数名，类型，默认值， 描述，组（没有用）
		
	argp.add_argument("a","aaa", TYPEID(string), "123", "this is test val", "group1");
	argp.add_argument("d","ddd", TYPEID(bool), "123.88", "this is test val", "group1");
	
	
	解析
	argp.analyze(argc, args);
	
	获取参数值，当类型不同，或是查找失败时，显示help
	// argp.add_argument();
	cout<<argp.get<int>("a")<<endl;
	cout<<argp.get<bool>("d")<<endl;
