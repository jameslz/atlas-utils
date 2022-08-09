#include "utils.h"

int orient_main(int argc, char *argv[]){
  
    int discard = 1;
    int c;
    while ((c = getopt(argc, argv, "r")) >= 0) {
        if(c == 'r') discard = 0;
    }

    if ( optind == argc || argc != optind + 2) {

        fprintf(stderr, "\nUsage: atlas-utils orient [options] <db> <fasta/q>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "-r         retain squence without strand information. \n\n");
        return 1;
    } 

    khash_t(reg) *h;
    h = kh_init(reg);
    khint_t k;

    gzFile     fp;    
    fp = strcmp(argv[optind], "-")? gzopen(argv[ optind ], "r") : gzdopen(fileno(stdin), "r"); 
    if( fp ){

        kstream_t *ks;
        int ret;
        ks            = ks_init(fp);
        kstring_t  kt  = {0, 0, 0};

        char *p;
        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            if( kt.l == 0 ) continue;
          
            ks_tokaux_t aux;
            p = kstrtok(kt.s, "\t", &aux);
            kt.s[aux.p - p] = '\0';

            k = kh_put(reg, h, kt.s, &ret);
            kh_key(h, k) = strdup(p);
            kh_val(h, k) = strdup(aux.p + 1);


        }
        free(kt.s);
        ks_destroy(ks);
        gzclose(fp);
    
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[ optind ]);
        exit(-1); 
    }

    fp = strcmp(argv[optind + 1], "-")? gzopen(argv[optind + 1], "r") : gzdopen(fileno(stdin), "r");
    
    if( fp ){
        
        kseq_t *kseq;
        kseq = kseq_init(fp);
        
        int l = 0;
        while ((l = kseq_read(kseq)) >= 0){
           
            k = kh_get(reg, h, kseq->name.s);
            if(k == kh_end(h) ){

                if( discard == 0 )
                    print_kseq(kseq, stdout);
                continue;

           }
           
           if(kh_val(h, k)[0] == '-'){
                rev_com( &kseq->seq);
                if ( kseq->qual.l )  rev( &kseq->qual);

            }

           print_kseq(kseq, stdout);

        }

        kseq_destroy(kseq);
        gzclose(fp);
    
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind + 1]);
        exit(-1); 
    }

    kh_reg_destroy(h);
    return 0;

}