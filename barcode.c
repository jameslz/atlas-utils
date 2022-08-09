#include "utils.h"

int barcode_main (int argc, char *argv[]){

    int len = 8;
    int c;
    while ((c = getopt(argc, argv, "l:")) >= 0) {
        if (c == 'l') len = atoi(optarg);
    }

    if ( optind == argc || argc != optind + 2) {
        fprintf(stderr, "\nUsage: atlas-utils barcode [options] <1.fq> <2.fq>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -l INT barcode length.\n\n");
        return 1;
    }

    gzFile fp_fv, fp_rv;
    fp_fv = strcmp(argv[ optind ], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r"); 
    fp_rv = strcmp(argv[ optind + 1], "-")? gzopen(argv[optind + 1], "r") : gzdopen(fileno(stdin), "r"); 

    if(fp_fv && fp_rv){

        kseq_t *kseq[2];
        kseq[0] = kseq_init(fp_fv);
        kseq[1] = kseq_init(fp_rv);

        while (kseq_read(kseq[0]) >= 0) {
            
            if (kseq_read(kseq[1]) < 0) {
                fprintf(stderr, "[W::%s] the 2nd file has fewer records.\n", __func__);
                break;
            }

            if(len > kseq[0]->seq.l || len > kseq[1]->seq.l){
                fprintf(stderr, "[W::%s] subseq length longer...\n", __func__);
                break;
            }

            kseq[0]->seq.s[len] = '\0';
            kseq[1]->seq.s[len] = '\0';

            printf("%s\t%s\t%s\n", kseq[0]->name.s, kseq[0]->seq.s, kseq[1]->seq.s);


        }
        
        if (kseq_read(kseq[1]) >= 0)
            fprintf(stderr, "[W::%s] the 1st file has fewer records.\n", __func__);
        
        kseq_destroy(kseq[0]); gzclose(fp_fv);
        kseq_destroy(kseq[1]); gzclose(fp_rv);
    }
    
    return 0;

}
