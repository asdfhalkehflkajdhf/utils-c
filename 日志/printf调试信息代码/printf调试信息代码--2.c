	  

	  
//写入一个固定的文件里
#define DEBUG(str, args...)  \
	{\
		FILE *tfp;\
		if ((tfp = fopen("./debug.log", "a")) != NULL)\
		{\
			   fprintf(tfp, str, ##args);\
			   fclose(tfp);\
		}\
	}
