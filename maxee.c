#include <math.h>
#include "utils.h"

int maxee_main(int argc, char *argv[]){
    
    int   offset = 33;
    float maxee  = 2;

    int c;
    while ((c = getopt(argc, argv, "q:E:")) >= 0) {
        if (c == 'q') offset = atoi(optarg);
        if (c == 'E') maxee  = atof(optarg);
    }

    if ( optind == argc || argc > optind + 1) {
        
        fprintf(stderr, "\nUsage: atlas-utils maxee [options] <in.fq>\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -q INT     offset value: 33 for Sanger quality, 64 for Illumina Phred+64 quality, default: 33\n");
        fprintf(stderr, "  -E FLOAT   Discard reads >E expected errors. \n\n");
        return 1;
    }

    int i;
    double eerrors[256] = {0.0};
    for ( i = 0; i <  128; ++i) eerrors[i + offset] =  pow(10., -i/10.0);

    gzFile fp;
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r");

    if(fp){
        
        kseq_t *kseq;
        kseq = kseq_init(fp);

        int l;
        double ee = 0.0;
        while ((l = kseq_read(kseq)) >= 0){

            for (ee=0., i = 0; i < kseq->seq.l; ++i) ee += eerrors[(int)kseq->qual.s[i]];
            if(ee > maxee) continue;
            print_kseq(kseq, stdout);

        }

        kseq_destroy(kseq);
        gzclose(fp);

     }else{

         fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind]);
         exit(-1);
     
     }

    return 0;
}