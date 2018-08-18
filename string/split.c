
typedef struct string_segmentation_struct
{
    char** strings;
    u32 len;
    char* internal_buf;
}string_segmentation_s;

void string_segmentation(s8 delimiter, s8 *string, s32 limit, string_segmentation_s * string_array)
{   
    u32 count = 1;
    s8 *pchar, **ptr;

	if ( NULL != string_array ) {
		memset(string_array, 0, sizeof(string_segmentation_s));
	}
    
    if(NULL == string || NULL == string_array || string[0] == '\0')
    {
        return;
    }
    
    if (0 == limit)
    {
        // È«²¿·Ö¸î
        limit = 99999;
    }
    
    string_array->internal_buf = strdup(string);
    if(NULL == string_array->internal_buf)
    {
        return;
    }
    
    pchar = string;
    while('\0' != *pchar && (s32)count < limit)
    {
        if (delimiter == *pchar)
        {
            count++;
        }
        pchar++;
    }
    string_array->strings = (s8**)malloc(count*sizeof(s8*));
    if(NULL == string_array->strings)
    {
        return;
    }
    string_array->len = count;
    
    ptr = string_array->strings;
    *ptr = string_array->internal_buf;
    pchar = string_array->internal_buf;
    while('\0' != *pchar && count > 1)
    {
        if (delimiter == *pchar)
        {
            ptr++;
            *ptr = pchar+1;
            *pchar = '\0';
            count--;
        }
        pchar++;
    }
}

void string_segmentation_free(string_segmentation_s *string_array)
{   
    if(NULL == string_array)
    {
        return;
    }
    if(string_array->internal_buf)
    {
        free(string_array->internal_buf);
    }
    if(string_array->strings)
    {
        free(string_array->strings);
    }
}
