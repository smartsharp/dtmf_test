/*
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Eavoo Inc. (C) 2013
 * 
 *  @file      base64.c
 *  @version   1.0
 *  @brief     
 *  @create    2013/8/26  hongbiao.zhang  file created.
 *  @revision  
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//#include <android/log.h>


#include "base64.h"



#define TAG "base64"
#define dbprint(fmt, arg...)  // __android_log_print(ANDROID_LOG_DEBUG, TAG, fmt, ##arg)




static const char BASE_CODE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";






/*************************************************************************
*  Function :        buffer_to_int 
*  Author:          hongbiao.zhang 
*  Version:         1.0        
*  Date:            2013-8-26     
*  Dependency:                 
*  Description:
*              1.              
*              2.              
*  Input:      
*              1.  char* buffer
*              2.  int len
*  return:    
*              static int                                                          
**************************************************************************/
static int buffer_to_int(char* buffer, int len)
{
    char c, index;
    int result = 0;
    
    if (!buffer) return result;

    for (index=0; index<len; index++)
    {
        c = buffer[index];

        if (c>'9' || c<'0')
            return 0;
            
        result = result*10 + (c - '0');
    }

    return result;
}




/*************************************************************************
*  Function :        base64_index 
*  Author:          hongbiao.zhang 
*  Version:         1.0        
*  Date:            2013-8-26     
*  Dependency:                 
*  Description:
*              1.              
*              2.              
*  Input:      
*              1.  unsigned char c
*  return:    
*              static int                                                          
**************************************************************************/
static int base64_index(unsigned char c) 
{   
    if ((c >= 'A') && (c <= 'Z'))  
    {   
        return c - 'A';  
    }
    else if ((c >= 'a') && (c <= 'z'))  
    {   
        return c - 'a' + 26;  
    }
    else if ((c >= '0') && (c <= '9'))  
    {   
        return c - '0' + 52;  
    }
    else if (c == '+')  
    {   
        return 62;  
    }
    else if (c == '/')  
    {   
        return 63;  
    }
    else if(c == '=')  
    {   
        return 0;  
    }
    
    return 0;  
}  


/*************************************************************************
*  Function :        base64_decode 
*  Author:          hongbiao.zhang 
*  Version:         1.0        
*  Date:            2013-8-26     
*  Dependency:                 
*  Description:
*              1.              
*              2.              
*  Input:      
*              1.  unsigned char* base64_in
*              2.  int len
*              3.  unsigned char* bytes_out
*  return:    
*              int                                                          
**************************************************************************/
int base64_decode(unsigned char* base64_in, int len, unsigned char* bytes_out) 
{
	int bin[3] = {0};
	int aux[4] = {0};
	int iter;
	int resto;
	int i = 0;
	int nzero = 0;
	int result;
	int oldlen = len;
	 
	for (i=0; i<len; i++) 
	{
		if (base64_in[i] == '=') 
		{
			len = i;
			break;
		}
	}
	iter = (len)/4;
	resto = (len)%4;

	if (resto != 0) nzero = 1;	
	result = iter * 3 + nzero * (resto - 1);
	
	for (i=0; i<iter; i++) 
	{
		aux[0] = base64_index(base64_in[i * 4]);
		aux[1] = base64_index(base64_in[i * 4 + 1]);
		aux[2] = base64_index(base64_in[i * 4 + 2]);
		aux[3] = base64_index(base64_in[i * 4 + 3]);

		bin[0] = ((aux[0] << 2) & 0xFC) | ((aux[1] >> 4)&0x03);
		bin[1] = ((aux[1]  << 4) &0xF0) | ((aux[2] >> 2) & 0x0F);
		bin[2] = ((aux[2]  << 6) & 0xC0) |  (aux[3] & 0x3F);
		 
		bytes_out[i * 3] = (char) bin[0];
		bytes_out[i * 3 + 1] = (char) bin[1];
		bytes_out[i * 3 + 2] = (char) bin[2];
	}
	 
	if (i == iter) 
	{	
		if (resto == 0) 
		{
		}

		if (resto == 2) 
		{
			aux[0] = base64_index(base64_in[i * 4]);
			aux[1] = base64_index(base64_in[i * 4 + 1]);
			 
			bin[0] = ((aux[0] << 2) & 0xFC) | ((aux[1] >> 4)&0x03);
			 
			bytes_out[i * 3] = (char) bin[0];
		}
		 
		if (resto == 3) 
		{
			aux[0] = base64_index(base64_in[i * 4]);
			aux[1] = base64_index(base64_in[i * 4 + 1]);
			aux[2] = base64_index(base64_in[i * 4 + 2]);
			 
			bin[0] = ((aux[0] << 2) & 0xFC) | ((aux[1] >> 4)&0x03);
			bin[1] = ((aux[1]  << 4) &0xF0) | ((aux[2] >> 2) & 0x0F);
			 
			bytes_out[i * 3] = (char) bin[0];
			bytes_out[i * 3 + 1] = (char) bin[1];
		}
	}

	//dbprint("base64_decode: in=%d, out=%d", oldlen, result);

	return result;
}

/*************************************************************************
*  Function :        base64_encode 
*  Author:          hongbiao.zhang 
*  Version:         1.0        
*  Date:            2013-8-26     
*  Dependency:                 
*  Description:
*              1.              
*              2.              
*  Input:      
*              1.  unsigned char* bytes_in
*              2.  int len
*              3.  unsigned char* base64_out
*  return:    
*              int                                                          
**************************************************************************/
int base64_encode(unsigned char* bytes_in, int len, unsigned char* base64_out)   
{  
	char bin[3] = {0};
	int aux[4] = {0};
	int iter = (len) / 3;
	int nzero = (len) % 3;
	int i = 0, j = 0, k = 0;
	
	for (i = 0, j=0; i<iter; i++) 
	{
        bin[0] = bytes_in[i * 3];
        bin[1] = bytes_in[i * 3 + 1];
        bin[2] = bytes_in[i * 3 + 2];

        aux[0] = ((bin[0] >> 2) & 0x3F);
        aux[1] = ((bin[0] << 4)&0x30) | ((bin[1] >> 4) & 0x0F);
        aux[2] = ((bin[1] << 2)&0x3C) | ((bin[2] >> 6) & 0x03);
        aux[3] = (bin[2] & 0x3F); 

        for (k=0; k<4; k++) 
        {
            base64_out[j++] = BASE_CODE[aux[k]];
        }
	}
	 
	if (i == iter) 
	{
		if (nzero == 0) 
		{
		} 
		else if (nzero == 1) 
		{ 
			aux[0] = ((bytes_in[iter * 3] >> 2) & 0x3F);
			aux[1] = (bytes_in[iter * 3]  << 4)& 0x30;
			 
			base64_out[j++] = BASE_CODE[aux[0]];
			base64_out[j++] = BASE_CODE[aux[1]];
			base64_out[j++] = '=';
			base64_out[j++] = '=';
		} 
		else if (nzero == 2) 
		{ 
			aux[0] = ((bytes_in[iter * 3] >> 2) & 0x3F);
			aux[1] = ((bytes_in[iter * 3]  << 4)&0x30) | ((bytes_in[iter * 3 + 1] >> 4) & 0x0F);
			aux[2] = (bytes_in[iter * 3 + 1]  << 2)&0x3C;
			 
			base64_out[j++] = BASE_CODE[aux[0]];
			base64_out[j++] = BASE_CODE[aux[1]];
			base64_out[j++] = BASE_CODE[aux[2]];
			base64_out[j++] = '=';
		}
	}

	base64_out[j] = '\0';
	//dbprint("base64_encode: in=%d, out=%d", len, j);

	return j;
}







