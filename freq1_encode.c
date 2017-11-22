
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include "misc.h"
//#include "base64.h"

static void usage(char *name) {
	printf("usage: %s hexstrings OutputBytesFile \n", name);
	printf("e.g    %s 0123456789abcdef dtmf.out\n", name);
	exit(1);
}

int sync_wave(int freq1, int freq2, int duration, int sr, int window, double *outbuf) {
	double phaseincr = 2.0 * M_PI * freq1 / sr;
	double phase = 0;

	int length = duration * sr / 1000;
	if (length > 16384) length = 16384;

	if(freq1 == freq2) {
		for (int i = 0; i < length; i++) {
			outbuf[i] = sin(phase);
			phase += phaseincr;
		}
	}else {
		for (int i = 0; i < length; i++) {
			outbuf[i] = 0.5*sin(phase);
			phase += phaseincr;
		}

		phaseincr = 2.0 * M_PI * freq2 / sr;
		phase = 0;
		for (int i = 0; i < length; i++) {
			outbuf[i] += 0.5 * sin(phase);
			phase += phaseincr;
		}		
	}
	
	switch(window){
	case 1: //hamming 
		for (int i = 0; i < length; i++) {
			outbuf[i] *= hamming(i*((double)1/(length-1)));
		}
		break;
	case 2: //hanning		
		for (int i = 0; i < length; i++) {
			outbuf[i] *= hanning(i*((double)1/(length-1)));
		}
		break;
	case 3: //black
		for (int i = 0; i < length; i++) {
			outbuf[i] *= blackman(i*((double)1/(length-1)));
		}
		break;	
	}
	return length;
}
int main(int argc, char *argv[])
{
    FILE         *fout;
    char *       input_str = NULL;
    int output_std = 0;
    int sr = 8000;
    int duration = 20;
    int sleep = 20;
    int total_len = 0;
    int sleep_len = sleep * sr / 1000;;
    


	input_str = argv[1];
    if (strcmp(argv[2], "-") == 0) { output_std=1; fout = stdout;}
    else if ( (fout = fopen(argv[2],"wb")) == NULL ) {
	fprintf(stderr, "Error opening output bytes file: %s: %s.\n",
         argv[2], strerror(errno));
	exit(1);
	}	
	
	double *outbufd = malloc(sizeof(double)*1600);	
	short *outbuf = malloc(sizeof(short)*1600);	
	int slen = strlen(input_str);	
	//slen = base64_encode((unsigned char *)input_str, slen, (unsigned char *)b64buf);	
	for(int i=0;i<slen;i++){
		int ch = input_str[i];
		int l = (ch>>4)&0xf;
		int r = ch&0xf;
		int freq1 = ((l+16)*100)+300;
		int freq2 = (r*100)+300;
		int len = sync_wave(freq1, freq2, duration, sr, 1,  outbufd);
		if (strcmp(argv[2], "-") != 0){
			printf("sync_wave ch='%c'/%d/0x%x freq1=%d, freq2=%d, duration=%d,len=%d\n", ch, ch, ch, freq1, freq2, duration, len);
		}
		for(int j=0; j<len;j++){
			outbuf[j] = outbufd[j]*32767;
		}
		// add sleep
		for(int j=0; j<sleep_len;j++){
			outbuf[len+j] = 0;
		}	
		len += sleep_len;
	    fwrite(outbuf, sizeof(short), len, fout);	    
		fflush(fout);
		total_len += len;
	}
	// add sleep
	for(int j=0; j<sleep_len;j++){
		outbuf[j] = 0;
	}	
    fwrite(outbuf, sizeof(short), sleep_len, fout);	    
	fflush(fout);
	
    if(!output_std)printf("dtmf_send %s = %d\n", input_str, total_len);
    fclose(fout);
    free(outbufd);
    free(outbuf);

    return 0;
}


