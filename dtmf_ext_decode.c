
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include "dtmf_ext.h"

static void usage(char *name) {
	printf("usage: %s InputShortsFile \n", name);
	printf("e.g    %s dtmf.encode\n", name);
	exit(1);
}


int main(int argc, char *argv[])
{
    FILE  *fin;
    char   outbuf[256];
    char   outbuf2[256]={0};
    int rdlen = 0;
    short *inbuf;
    int from_stdin = 0;
    int duration = (DEFAULT_FRAMESIZE/80)*10;

    if (argc < 2) {
		usage(argv[0]);
		return -1;
    }
    if (strcmp(argv[1], "-") == 0) { from_stdin=1; fin = stdin;}
    else if ( (fin = fopen(argv[1],"rb")) == NULL ) {
	fprintf(stderr, "Error opening input shorts file: %s: %s.\n",
         argv[1], strerror(errno));
	exit(1);
    }
    dtmf_debug = !from_stdin;
	inbuf = malloc(sizeof(short)*DEFAULT_FRAMESIZE);	
	dtmf_ext_init(8000, duration, duration*4);	
	//fread(inbuf, sizeof(short), DEFAULT_FRAMESIZE/2, fin);
	while((rdlen = fread(inbuf, sizeof(short), DEFAULT_FRAMESIZE, fin))>0){
		outbuf[0] = 0;
		if (!from_stdin) printf("read %d\n", rdlen);
    	int len = dtmf_ext_receive(inbuf, rdlen, outbuf);
    	if(len > 0){
    		strcat(outbuf2, outbuf);
    	}
        if (fin == stdin) fflush(stdin);
    }
    dtmf_ext_deinit();
    fclose(fin);
    free(inbuf);
    printf("decode: %s\n", outbuf2);

    return 0;
}


