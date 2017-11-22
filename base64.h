/*
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of EAVOO Inc. (C) 2013
 *
 *  @file      base64.h
 *  @version   1.0
 *  @brief     
 *  @create    2013/8/26  hongbiao.zhang  file created.
 *  @revision  
 *
 */


#ifndef __BASE64_H__
#define __BASE64_H__


#ifdef __cplusplus
extern "C" {
#endif






extern int base64_decode(unsigned char* base64_in, int len, unsigned char* bytes_out);

extern int base64_encode(unsigned char* bytes_in, int len, unsigned char* base64_out) ;












#ifdef __cplusplus
}
#endif



#endif /* __BASE64_H__ */
