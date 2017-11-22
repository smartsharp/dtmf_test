
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include "misc.h"
#include "kiss_fft.h"
#include "kiss_fftr.h"

static void usage(char *name) {
	printf("usage: %s InputShortsFile \n", name);
	printf("e.g    %s freq1.encode\n", name);
	exit(1);
}

static int trim_buf(short *buf, int len, int limit){
	int i,cnt=0;
	for(i=0;i<len;i++){
		if(buf[i]!=0){
			break;
		}
	}
	if(i>=limit){
		len -= limit;
		memmove(buf, &buf[i], len);
	}
	for(i=len-2;i>0;i--){
		if(buf[i]==0){
			cnt++;
		}else{
			break;
		}
	}
	if(cnt>=limit){
		len -= limit;
	}
	return len;
}

static int analysis_buf(float *inbuff, int length, int sr, kiss_fftr_cfg fft, kiss_fft_cpx *fft_in, kiss_fft_cpx *fft_out, int fft_size, char* outbuf){
	int i, f1, f2, ch, tag1, tag2;
	float max = 0, max1 = 0, max2 = 0;
	float freq, freq1 = 0, freq2 = 0;
	kiss_fft_scalar zero;
	memset(&zero,0,sizeof(zero));

	
	for (i = 0; i < length; i++) {
	    fft_in[i].r = inbuff[i];
	    fft_in[i].i = zero;
	}
	for (; i < fft_size; i++) {
	    fft_in[i].r = zero;
	    fft_in[i].i = zero;
    }		
	kiss_fftr(fft, (kiss_fft_scalar*) fft_in, fft_out);
 	//kiss_fftri(ifft, fft_out, (kiss_fft_scalar*)fft_reconstructed);
 	//calculate freq
 	max1 = 0;
 	freq1 = 0;
 	tag1 = -1;
	for (i = 0; i < fft_size; i++) {	
	    max = (float)abs(fft_out[i].r);
		freq = ((float)i/fft_size)*sr;
		//if(max>0) printf("i=%d, freq=%f, a=%f\n", i, freq, max);
		if(max>max1 && freq>1800 && freq<3500){
			max1 = max;
			freq1 = freq;
			tag1 = i;
		}
	}
	max2 = 0;
 	freq2 = 0;
 	tag2 = -1;
	for (i = 0; i < fft_size; i++) {	
	    max = (float)abs(fft_out[i].r);	
		freq = ((float)i/fft_size)*sr;
		if(i!=tag1 && max >max2 && freq>300 && freq<1900){
			max2 = max;
			freq2 = freq;
			tag2 = i;
		}
	}


	f1 = ((int)((freq1/100+0.5)))*100;
	f2 = ((int)((freq2/100+0.5)))*100;
	ch = 0;		
	if(f1>0 && f2>0){		
		ch = (((((int)((f1-300)/100))-16)&0xf)<<4)|(((int)((f2-300)/100))&0xf);	
		printf("freq1 %d, freq2 %d, ch='%c'\n", f1, f2,ch);
		*outbuf = ch;
		return 1;
 	}
 	return 0;
}

int main(int argc, char *argv[])
{
    FILE  *fin;
    int buflen = 0;
    short *inbuf;
    float *inbuff;
    int sr = 8000;
    int duration = 20;
	int frame_length = duration * sr / 1000;
	int from_stdin = 0;

    int fft_size = 256;
	//int isinverse = 1;
	kiss_fft_cpx *fft_in;
	kiss_fft_cpx *fft_out;
	//kiss_fft_cpx *fft_reconstructed;
	kiss_fftr_cfg fft, ifft;
	int i, rdlen;
	char ch;

    if (argc < 2) {
		usage(argv[0]);
		return -1;
    }
    if (strcmp(argv[1], "-") == 0) { from_stdin = 1; fin = stdin; }
    else if ( (fin = fopen(argv[1],"rb")) == NULL ) {
	fprintf(stderr, "Error opening input shorts file: %s: %s.\n",
         argv[1], strerror(errno));
	exit(1);
    }

    if (strcmp(argv[1], "-") != 0) printf("frame_length = %d, fft_size = %d\n", frame_length, fft_size);

	fft_in = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx)*fft_size);
	fft_out = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx)*fft_size);
	//fft_reconstructed = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx)*fft_size);

	fft = kiss_fftr_alloc(fft_size*2 ,0 ,0,0);
	//ifft = kiss_fftr_alloc(fft_size*2,isinverse,0,0);

	    
	inbuf = malloc(sizeof(short)*frame_length*4);	
	inbuff = malloc(sizeof(float)*frame_length*4);	
	//fread(inbuf, sizeof(short), frame_length/2, fin);	
	//if (!from_stdin) printf("skip leading %d\n", frame_length/2);
	while((rdlen = fread(&inbuf[buflen], sizeof(short), frame_length, fin))>0){
		if (!from_stdin) printf("rdlen %d, buflen %d\n", rdlen, buflen);
		buflen += rdlen;
		//buflen = trim_buf(inbuf, buflen, frame_length/2);
		if (!from_stdin) printf("buflen %d\n", buflen);
		if(buflen < (frame_length/2))	continue;

		if(buflen >= frame_length) {
			for(i=0;i<frame_length;i++){
				inbuff[i] = inbuf[i];
			}
			normalize2(inbuff, frame_length);
			for(i=0;i<frame_length;i++){
				inbuff[i] *= 32767;
			}
			if (!from_stdin) printf("analysis_buf\n");
			i = analysis_buf(inbuff, frame_length, sr, fft, fft_in, fft_out, fft_size, &ch);
			buflen -= frame_length;
			memmove(inbuf, &inbuf[frame_length], buflen);
			//buflen = trim_buf(inbuf, buflen, frame_length/2);
		}
    	
	 	
        if (fin == stdin) fflush(stdin);
    }

	
    fclose(fin);
    free(inbuf);
    free(inbuff);
    
    free(fft_in);
    free(fft_out);
    //free(fft_reconstructed);
    kiss_fft_free(fft);
    //kiss_fft_free(ifft);
    return 0;
}



