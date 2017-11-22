/*
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of EAVOO Inc. (C) 2017
 *
 *  @file      misc.h
 *  @version   1.0
 *  @brief     
 *  @create    2017/11/21  zhanghongbiao  file created.
 *  @revision  
 *
 */


#ifndef __MISC_H__
#define __MISC_H__

/*=============================================================*/
// Include files
/*=============================================================*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*=============================================================*/
// Extern definition
/*=============================================================*/

/*=============================================================*/
// Macro definition
/*=============================================================*/
#ifndef M_PI
#define M_PI (3.1415926535897932385)
#endif

typedef unsigned char bool;
#define true 1
#define false 0
/*=============================================================*/
// Data type definition
/*=============================================================*/

/*=============================================================*/
// Global function definition
/*=============================================================*/
double blackman(double x);
double hamming(double x);
double hanning(double x);

// Hamming - used by gmfsk
void HammingWindow(double *array, int n);
// Hanning - used by winpsk
void HanningWindow(double *array, int n);
// Best lob suppression - least in band ripple
void BlackmanWindow(double *array, int n);


int normalize(double *v, int n);
int normalize2(float *v, int n);

















#endif /* __MISC_H__ */
