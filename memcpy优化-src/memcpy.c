//多路比单路只快了5微秒左右
void memcpy2(char *to , char *from , int count){
	if(count == 0 ||to == NULL || from == NULL){
		return;
	}
	//找对齐Coefficient of alignment
	//int C_of_a = sizeof(void *);
	int C_of_a = sizeof(long int);
	//块复制
	int block = count/C_of_a;
	//剩余
	int surplus = count%C_of_a;
	switch (surplus ) {
		case 0:     *(to+7)=*(from+7);to++;from++; ;
		case 7:     *(to+6)=*(from+6);to++;from++; ;
		case 6:     *(to+5)=*(from+5);to++;from++; ;
		case 5:     *(to+4)=*(from+4);to++;from++; ;
		case 4:     *(to+3)=*(from+3);to++;from++; ;
		case 3:     *(to+2)=*(from+2);to++;from++; ;
		case 2:     *(to+1)=*(from+1);to++;from++; ;
		case 1:     *(to+0)=*(from+0);to++;from++; ;
    } 
	//
	while(--block>0){
		*(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a; ;
	}
	
}


void memcpy2(char *to , char *from , int count){
	if(count == 0 ||to == NULL || from == NULL){
		return;
	}
	//找对齐Coefficient of alignment
	//int C_of_a = sizeof(void *);
	int C_of_a = sizeof(long int);
	//块复制
	int block = count/C_of_a;
	//剩余
	int surplus = count%C_of_a;
	int double_b = block/8;
	int double_s = block%8;
	switch (surplus ) {
		case 7:     *(to+6)=*(from+6);to++;from++; ;
		case 6:     *(to+5)=*(from+5);to++;from++; ;
		case 5:     *(to+4)=*(from+4);to++;from++; ;
		case 4:     *(to+3)=*(from+3);to++;from++; ;
		case 3:     *(to+2)=*(from+2);to++;from++; ;
		case 2:     *(to+1)=*(from+1);to++;from++; ;
		case 1:     *(to+0)=*(from+0);to++;from++; ;
    } 
	switch (double_s ) {
		case 7:     *(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a;
		case 6:     *(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a;
		case 5:     *(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a;
		case 4:     *(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a;
		case 3:     *(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a;
		case 2:     *(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a;
		case 1:     *(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a;
    } 
	while(--double_b>0){
		*(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a; ;
		*(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a; ;
		*(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a; ;
		*(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a; ;
		*(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a; ;
		*(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a; ;
		*(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a; ;
		*(long int *)(to)=*(long int *)(from);to+=C_of_a;from+=C_of_a; ;
	}
	
}
