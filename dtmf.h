/*
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of EAVOO Inc. (C) 2017
 *
 *  @file      dtmf.h
 *  @version   1.0
 *  @brief     
 *  @create    2017/11/20  zhanghongbiao  file created.
 *  @revision  
 *
 */


#ifndef __DTMF_H__
#define __DTMF_H__

/*=============================================================*/
// Include files
/*=============================================================*/
#include "misc.h"
/*=============================================================*/
// Extern definition
/*=============================================================*/

/*=============================================================*/
// Macro definition
/*=============================================================*/
#define DEFAULT_FRAMESIZE 80 //160 

/*=============================================================*/
// Data type definition
/*=============================================================*/

/*=============================================================*/
// Global function definition
/*=============================================================*/

extern void dtmf_init(int sample_rate, int duration, int silence);
extern void dtmf_deinit();
extern int dtmf_send(char *ch, short *outbuf, int window);
extern int dtmf_custom_freq(int low, int high, short *outbuf, int window);
extern int dtmf_receive(const short* buf, int len, char * outbuf);
















#endif /* __DTMF_H__ */
