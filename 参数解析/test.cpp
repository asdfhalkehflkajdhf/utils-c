#include "argparse.hpp"

int main(int argc, char **args){


	argparse argp;
	
	argp.add_argument("a","aaa", TYPEID(string), "123", "this is test val", "group1");
	argp.add_argument("d","ddd", TYPEID(bool), "123.88", "this is test val", "group1");
	
	argp.analyze(argc, args);
	// argp.add_argument();
	cout<<argp.get<int>("a")<<endl;
	cout<<argp.get<bool>("d")<<endl;
	return 0;
}