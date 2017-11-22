/*
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Eavoo Inc. (C) 2017
 * 
 *  @file      dtmf.c
 *  @version   1.0
 *  @brief     
 *  @create    2017/11/20  zhanghongbiao  file created.
 *  @revision  
 * 
 */


#include "dtmf_ext.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <ctype.h>



typedef struct _goertzel{
	int N;
	int count;
	double Q0;
	double Q1;
	double Q2;
	double k1;
	double k2;
	double k3;
	bool isvalid;
}goertzel;

#define RANGE  0.5         /* any thing higher than RANGE*peak is "on" */
#define THRESH 1000        /* 6 dB s/n for detection */
#define FLUSH_TIME 10      /* 10 frames ~ 330 millisecond */


#define NUMTONES 12

// tones in 4x4x4 array
// 697  770  842  941  1209  1336  1447  1633

static int ind[] = {319, 406, 471, 587}; 
static int row[] = {697, 770, 852, 941};
static int col[] = {1209, 1336, 1477, 1633}; 

const char rc[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int SR = 8000;
static int RT = 0;
static double shape[128];
static int ch_duration = 0; //ms
static int ch_silence = 0; //ms

static double data[400];
static double power[NUMTONES];
//double thresh;
static double maxpower;
static double minpower;
static int framesize;
static int silence_time;
static int last;

static goertzel *filt[NUMTONES];
#define DEFAULT_FRAMESIZE_2 331

int dtmf_debug = 0;

static goertzel* new_goertzel(int n, double freq, double sr)
{
	double w;
	goertzel* g = (goertzel*)malloc(sizeof(goertzel));
	w = 2 * M_PI * freq / sr;
	g->k1 = cos(w);
	g->k2 = sin(w);
	g->k3 = 2.0 * g->k1;
	g->Q0 = g->Q1 = g->Q2 = 0.0;
	g->count = g->N = n;
}

void dtmf_ext_init(int sample_rate, int duration, int silence)
{
	for (int i = 0; i < 4; i++) {
		filt[i] = new_goertzel(DEFAULT_FRAMESIZE, ind[i], 8000);
		assert(filt[i]);
	}
	for (int i = 0; i < 4; i++) {
		filt[i+4] = new_goertzel(DEFAULT_FRAMESIZE, row[i], 8000);
		assert(filt[i+4]);
	}
	for (int i = 0; i < 4; i++) {
		filt[i+8] = new_goertzel(DEFAULT_FRAMESIZE, col[i], 8000);
		assert(filt[i+8]);
	}
	for (int i = 0; i < DEFAULT_FRAMESIZE; i++) data[i] = 0;
	framesize = DEFAULT_FRAMESIZE; // 8000 sr default
	silence_time = 0;
	last = ' ';
		
	SR = sample_rate;
	RT = (int)(sample_rate * 4 / 1000.0); // 4 msec edge
	ch_duration = duration;
	ch_silence = silence;
	if(ch_duration > 50) ch_duration = 50;
	if(ch_silence > 50) ch_silence = 50;
	for (int i = 0; i < 128; i++) shape[i] = 1.0;
	for (int i = 0; i < RT; i++)
		shape[i] = 0.5 * (1.0 - cos (M_PI * i / RT));
}

void dtmf_ext_deinit(){
	for (int i = 0; i < NUMTONES; i++) {
		if(filt[i] != NULL){
			free(filt[i]);
			filt[i] = NULL;
		}
	}
}


static int dtmf_ext_custom_freq(int ind, int low, int high, short *outbuf, int window)
{
	double phaseincr = 2.0 * M_PI * ind / SR;
	double phase = 0;
	int length = ch_duration * SR / 1000;
	//printf("dtmf_ext_custom_freq: low %d, high %d, length %d\n", low, high, length);
	if (length > 16384) length = 16384;

	for (int i = 0; i < length; i++) {
		data[i] = 0.25 * sin(phase);
		phase += phaseincr;
	}

	phaseincr = 2.0 * M_PI * low / SR;
	phase = 0;
	for (int i = 0; i < length; i++) {
		data[i] += 0.25 * sin(phase);
		phase += phaseincr;
	}

	phaseincr = 2.0 * M_PI * high / SR;
	phase = 0;
	for (int i = 0; i < length; i++) {
		data[i] += 0.25 * sin(phase);
		phase += phaseincr;
	}
	switch(window){
	case 1: //hamming 
		for (int i = 0; i < length; i++) {
			data[i] *= hamming(i*((double)1/(length-1)));
		}
		break;
	case 2: //hanning		
		for (int i = 0; i < length; i++) {
			data[i] = hanning(data[i]);
		}
		break;
	case 3: //black
		for (int i = 0; i < length; i++) {
			data[i] = blackman(data[i]);
		}
		break;
	case 0:	
		for (int i = 0; i < RT; i++) {
			data[i] *= shape[i];
			data[length - 1 - i] *= shape[i];
		}
		break;
	}
	for(int i=0;i<length;i++){
		outbuf[i] = (short)(data[i]*32767);
		//printf("dtmf_ext_custom_freq: %d, %f, %d\n", i, data[i], outbuf[i]);
	}

	return length;
}

static int dtmf_ext_silence(short *outbuf)
{
	int length = ch_silence * SR / 1000;
	if (length > 16384) length = 16384;
	//printf("dtmf_ext_silence\n");
	memset(outbuf, 0, length * sizeof(short));
	return length;
}

static int dtmf_ext_two_tone(int ch, short *outbuf, int window)
{	
	if (!strchr(rc, ch)) return 0;
	int pos = strchr(rc, ch) - rc;
	int p = pos /16;
	int r = (pos%16) / 4;
	int c = (pos%16) % 4;
	//printf("dtmf_ext_two_tone: %c, pos=%d, p=%d, r=%d, c=%d\n", ch, pos, p, r, c);
	return dtmf_ext_custom_freq(ind[p], row[r], col[c], outbuf, window);
}

int dtmf_ext_send(char *ch, short *outbuf, int window){
	int len = strlen(ch);
	int out_len = 0;
	if(SR <= 0 || ch_duration <= 0 || ch_silence <=0) return 0;
	for(int i = 0; i < len; i++) {
		int c = ch[i];
		if (c == ' ' || c == ',' || c == '-') {
			out_len += dtmf_ext_silence(&outbuf[out_len]);
		}else {
			out_len += dtmf_ext_two_tone(c, &outbuf[out_len], window);
		}
		out_len += dtmf_ext_silence(&outbuf[out_len]);
	}
	return out_len;
}


//======================================================================
// DTMF tone receive
//======================================================================
// tone #s and coefficients
// 8000 Hz sampling N == 240
// 11025 Hz sampling N == 331

static void goertzel_reset(goertzel *g, int n, double freq, double sr)
{
	double w;
	w = 2 * M_PI * freq / sr;
	g->k1 = cos(w);
	g->k2 = sin(w);
	g->k3 = 2.0 * g->k1;
	g->Q0 = g->Q1 = g->Q2 = 0.0;
	g->count = g->N = n;
	g->isvalid = false;
}

static bool goertzel_run(goertzel *g, double sample)
{
    g->Q0 = sample + g->k3*g->Q1 - g->Q2;
    g->Q2 = g->Q1;
    g->Q1 = g->Q0;
    if (--g->count == 0) {
	g->count = g->N;
	return true;
    }
    return false;
}

static double goertzel_real(goertzel *g)
{
    return ((0.5*g->k3*g->Q1 - g->Q2)/g->N);
}

static double goertzel_imag(goertzel *g)
{
    return ((g->k2*g->Q1)/g->N);
}

static double goertzel_mag(goertzel *g)
{
    return (g->Q2*g->Q2 + g->Q1*g->Q1 - g->k3*g->Q2*g->Q1);
}


/*
 * calculate the power of each tone using Goertzel filters
 */
static void calc_power()
{
// reset row freq filters
	for (int i = 0; i < 4; i++) goertzel_reset(filt[i], DEFAULT_FRAMESIZE, ind[i], SR);
// reset row freq filters
	for (int i = 0; i < 4; i++) goertzel_reset(filt[i+4], DEFAULT_FRAMESIZE, row[i], SR);
// reset col freq filters
	for (int i = 0; i < 4; i++) goertzel_reset(filt[i+8], DEFAULT_FRAMESIZE, col[i], SR);

	for (int i = 0; i < framesize; i++)
		for (int j = 0; j < NUMTONES; j++)
			goertzel_run(filt[j], data[i]);

	for (int i = 0; i < NUMTONES; i++)
		power[i] = goertzel_mag(filt[i]);

	maxpower = 0;
	minpower = 1e6;
	for (int i = 0; i < NUMTONES;i++) {
		if (power[i] > maxpower)
			maxpower = power[i];
		if (power[i] < minpower)
			minpower = power[i];
	}
	if ( minpower == 0 ) minpower = 1e-3;
}

static int decode()
{ 
	calc_power();

	if (maxpower < (10 * 5.0/*progStatus.sldrSquelchValue*/)){
		return ' ';
	}
	//printf("decode: maxpower=%d\n", maxpower);

	int p = -1, r = -1, c = -1;
	double pwr = 0;
	for (int i = 0; i < 4; i++)
		if (power[i] > pwr) {
			pwr = power[i];
			p = i;
		}
	pwr = 0;
	for (int i = 0; i < 4; i++)
		if (power[i+4] > pwr) {
			pwr = power[i+4];
			r = i;
		}
	pwr = 0;
	for (int i = 0; i < 4; i++)
		if (power[i+8] > pwr) {
			pwr = power[i+8];
			c = i;
		}
	if (p==-1 || r == -1 || c == -1) 
		return ' ';
	//printf("decode p=%d, r=%d, c=%d\n", p, r, c);
	return rc[p*16+r*4 + c];
}

int dtmf_ext_receive(const short* buf, int len, char * outbuf)
{
	char dtmfchars[250]={0};
	int x;
	static int dptr = 0;
	int bufptr = 0;

	int framesize = (SR == 8000) ? DEFAULT_FRAMESIZE : DEFAULT_FRAMESIZE_2;
	int dtmfchars_len = 0;
	int outlen = 0;
	if(dtmf_debug) printf("dtmf_ext_receive: len=%d\n", len);

	while (1) {
		int i;
		for ( i = dptr; i < framesize; i++) {
			data[i] = buf[bufptr];
			bufptr++;
			if (bufptr == len) break;
		}
		if (i < framesize) {
			dptr = i + 1;
			if(dtmfchars_len > 0){
				dtmfchars[dtmfchars_len] = 0;
				strcpy(&outbuf[outlen], dtmfchars);
				outlen += dtmfchars_len;
			}
			return outlen;
		}
		dptr = 0;
		normalize(data, framesize);

		x = decode(); 
		if(dtmf_debug) printf("decode: x='%c', last='%c', silence_time=%d\n", x, last, silence_time);
		if (x == ' ') {
			silence_time++;
			if (silence_time == 4 && dtmfchars_len>0) dtmfchars[dtmfchars_len++]= ' ';
			if (silence_time == FLUSH_TIME) {
				if (dtmfchars_len>0) {
					dtmfchars[dtmfchars_len] = 0;
					strcpy(&outbuf[outlen], dtmfchars);
					outlen += dtmfchars_len;
					dtmfchars_len = 0;
				}
				silence_time = 0;
			}
		} else {
			silence_time = 0;
			if (x != last && last == ' ')
				dtmfchars[dtmfchars_len++] = x;
		}
		last = x;
	}
}













