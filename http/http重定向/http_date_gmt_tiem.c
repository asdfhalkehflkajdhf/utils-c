#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<time.h>

int main()
{
	char date_gmt[35] = {0};
	struct tm *ptr;
	time_t timep;
	time(&timep);
	ptr = localtime(&timep);
	strftime(date_gmt,35,"%a, %d %h %Y %T GMT",ptr);
	
	printf("%s\n",date_gmt);
	return 0 ;
}