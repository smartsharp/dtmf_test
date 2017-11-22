/*
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Eavoo Inc. (C) 2017
 * 
 *  @file      misc.c
 *  @version   1.0
 *  @brief     
 *  @create    2017/11/21  zhanghongbiao  file created.
 *  @revision  
 * 
 */


#include "misc.h"


double blackman(double x)
{
	return (0.42 - 0.50 * cos(2 * M_PI * x) + 0.08 * cos(4 * M_PI * x));
}

double hamming(double x)
{
	return 0.54 - 0.46 * cos(2 * M_PI * x);
}

double hanning(double x)
{
	return 0.5 - 0.5 * cos(2 * M_PI * x);
}

// Hamming - used by gmfsk
void HammingWindow(double *array, int n) {
	double pwr = 0.0;
	double inv_n = 1.0 / (double)n;
	for (int i = 0; i < n; i++) {
		array[i] = hamming((double)i * inv_n);
		pwr += array[i] * array[i];
	}
	pwr = sqrt((double)n/pwr);
	for (int i = 0; i < n; i++)
		array[i] *= pwr;
}

// Hanning - used by winpsk
void HanningWindow(double *array, int n) {
	double pwr = 0.0;
	double inv_n = 1.0 / (double)n;
	for (int i = 0; i < n; i++) {
		array[i] = hanning((double)i * inv_n);
		pwr += array[i] * array[i];
	}
	pwr = sqrt((double)n/pwr);
	for (int i = 0; i < n; i++)
		array[i] *= pwr;
}

// Best lob suppression - least in band ripple
void BlackmanWindow(double *array, int n) {
	double pwr = 0.0;
	double inv_n = 1.0 / (double)n;
	for (int i = 0; i < n; i++) {
		array[i] = blackman((double)i * inv_n);
		pwr += array[i] * array[i];
	}
	pwr = sqrt((double)n/pwr);
	for (int i = 0; i < n; i++)
		array[i] *= pwr;
}



int normalize(double *v, int n)
{
	if( n == 0 ) return 0 ;

	double max = v[0];
	double min = v[0];
	int j;

	/* find max and min values */
	for (j=1; j<n; j++) {
		float vj = v[j];
		if (vj > max)	max = vj;
		else if (vj < min)	min = vj;
	}
	/* all values 0 - no need to normalize or decode */
	if (max == 0.0) return 0;

	float ratio = 1.0/max;
	for (j=0; j<n; j++) v[j] *= ratio;
	return (1);
}


int normalize2(float *v, int n)
{
	if( n == 0 ) return 0 ;

	float max = v[0];
	float min = v[0];
	int j;

	/* find max and min values */
	for (j=1; j<n; j++) {
		float vj = v[j];
		if (vj > max)	max = vj;
		else if (vj < min)	min = vj;
	}
	/* all values 0 - no need to normalize or decode */
	if (max == 0.0) return 0;

	float ratio = 1.0/max;
	for (j=0; j<n; j++) v[j] *= ratio;
	return (1);
}


















