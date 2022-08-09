#include "utils.h"

int linkpairs_main (int argc, char *argv[]){
    
    int ns = 8;
    
    int c;
    while ((c = getopt(argc, argv, "n:")) >= 0) {
        if(c == 'n') ns = atoi(optarg);
    }

    if ( optind == argc || argc > optind + 2) {
        fprintf(stderr, "\nUsage: atlas-utils linkpairs [options] <in1.fq> <in2.fq>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -n  INT  inserting padding number of 'N's, default:[8] \n\n");
        return 1;
    }    

    gzFile fp1, fp2;
    kseq_t *kseq[2];

    fp1 = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r");
    fp2 = strcmp(argv[optind+1], "-")? gzopen(argv[optind+1], "r") : gzdopen(fileno(stdin), "r");
    
    
    if(fp1 && fp2){

        kseq[0] = kseq_init(fp1);
        kseq[1] = kseq_init(fp2);
        
        int i;

        while (kseq_read(kseq[0]) >= 0) {
          
            if (kseq_read(kseq[1]) < 0) {
                fprintf(stderr, "[W::%s]: the 2nd file has fewer records.\n", __func__);
                break;
            }

            if( kseq[0]->name.s[ kseq[0]->name.l - 2 ] == '/' ){
                kseq[0]->name.s[ kseq[0]->name.l - 2 ] = '\0';
                kseq[0]->name.l  -= 2;
            }

            rev_com( &kseq[1]->seq);
            rev( &kseq[1]->qual);

            if(ns == 0){
                 printf("@%s\n%s%s\n+\n%s%s\n", kseq[0]->name.s,
                                                kseq[0]->seq.s , kseq[1]->seq.s,
                                                kseq[0]->qual.s, kseq[1]->qual.s);   
            }else{

                printf("@%s\n%s", kseq[0]->name.s, kseq[0]->seq.s);
                for (i = 0; i < ns; ++i) putchar('N');
                printf("%s\n+\n", kseq[1]->seq.s); 
                
                if(kseq[0]->qual.l > 0){
    	            printf("%s", kseq[0]->qual.s);
    	            for (i = 0; i < ns; ++i) putchar('I');
    	            printf("%s\n", kseq[1]->qual.s);
    	   		}

            }

        }

        if (kseq_read(kseq[1]) >= 0)
            fprintf(stderr, "[W::%s]: the 1st file has fewer records.\n", __func__);
        
        kseq_destroy(kseq[0]); gzclose(fp1);
        kseq_destroy(kseq[1]); gzclose(fp2);
    }else
       fprintf(stderr, "[ERR]: can't open file %s || %s \n", argv[optind], argv[optind + 1]);

    return 0;
}
