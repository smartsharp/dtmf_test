
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include "dtmf.h"

static void usage(char *name) {
	printf("usage: %s hexstrings OutputBytesFile \n", name);
	printf("e.g    %s 0123456789abcdef dtmf.out\n", name);
	exit(1);
}


int main(int argc, char *argv[])
{
    FILE         *fout;
    int cust_hf=0, cust_lf = 0;
    char *       input_str = NULL;
    int output_std = 0;
    int duration = (DEFAULT_FRAMESIZE/80)*10;


    if (argc < 2) {
		usage(argv[0]);
		return -1;
    }

    if (strcmp(argv[1], "-f")  == 0) {   
    	if(argc <5){
			usage(argv[0]);
			return -1;
		}
		cust_lf = atoi(argv[2]);
		cust_hf = atoi(argv[3]);
		if(cust_lf <= 0 || cust_hf <=0 || cust_lf >= cust_hf){
			usage(argv[0]);
			return -1;
		}
		if (strcmp(argv[4], "-") == 0) { output_std=1; fout = stdout;}
	    else if ( (fout = fopen(argv[2],"wb")) == NULL ) {
		fprintf(stderr, "Error opening output bytes file: %s: %s.\n",
	         argv[2], strerror(errno));
		exit(1);
	    }
    }else {
    	input_str = argv[1];
	    if (strcmp(argv[2], "-") == 0) { output_std=1; fout = stdout;}
	    else if ( (fout = fopen(argv[2],"wb")) == NULL ) {
		fprintf(stderr, "Error opening output bytes file: %s: %s.\n",
	         argv[2], strerror(errno));
		exit(1);
	    }
    }
	short *outbuf = malloc(sizeof(short)*strlen(input_str)*1600);	
	
	dtmf_init(8000, duration, duration);	
	
    int len = dtmf_send(input_str, outbuf, 1);
    dtmf_deinit();
    if(!output_std)printf("dtmf_send %s = %d\n", input_str, len);
    fwrite(outbuf, sizeof(short), len, fout);
	fflush(fout);
    fclose(fout);
    free(outbuf);

    return 0;
}


