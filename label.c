#include "utils.h"

int label_main(int argc, char *argv[]){
  
    int discard = 0;
    char *padding = " ";
    int c;
    while ((c = getopt(argc, argv, "p:r")) >= 0) {
        if (c == 'p') padding = optarg;
        else if(c == 'r') discard = 1;
    }

    if ( optind == argc || argc != optind + 2) {

        fprintf(stderr, "\nUsage: atlas-utils label [options] <db> <fasta>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -p string  string padding to headline [default ' '].\n");
        fprintf(stderr, "  -r         discard squence without label. \n\n");
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

    if(kh_size(h) == 0) return 0;

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
            
           ( kseq->qual.l ) ?  fputc('@', stdout) : fputc('>', stdout); 
            fprintf(stdout, "%s%s%s", kseq->name.s, padding, kh_val(h, k));
            ( kseq->qual.l ) ?  fprintf(stdout, "\n%s\n+\n%s\n", kseq->seq.s, kseq->qual.s):
                                fprintf(stdout, "\n%s\n", kseq->seq.s); 

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