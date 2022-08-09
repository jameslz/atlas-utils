#include "utils.h"

typedef struct{
  int start;
  int end;
  int strand;
} STRIP_T;

KHASH_MAP_INIT_STR(strip, STRIP_T *);

void kh_strip_destroy(khash_t(strip) *h){
    
    khint_t k;
    if  (h == 0) return;
    for (k = 0; k < kh_end(h); ++k){
        if (kh_exist(h, k)){
            free( (char*)kh_key(h, k) );
            free( (STRIP_T*)kh_val(h, k) );
        }
    }
    kh_destroy(strip, h);
}

int primer_strip_main (int argc, char *argv[]){
    
    int width = 20;
    int c;
    while ((c = getopt(argc, argv, "w:")) >= 0) {
        if (c == 'w') width = atoi(optarg);
    }

    if ( optind == argc || argc != optind + 2) {
        
        fprintf(stderr, "\nUsage: atlas-utils primer_strip <oligo> <fastq>\n\n");
        fprintf(stderr, "Note: usearch search oligodb parameters: query+qstrand+target+qlo+qhi+ql+diffs+tlo+thi\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -w INT max shifted width, default, [20]\n\n");
        return 1;
    
    }

    kstring_t kt = {0, 0, 0};
    int *fields, n, ret;

    khash_t(strip) *h;
    h = kh_init(strip);
    khint_t k;

    //query+qstrand+target+qlo+qhi+ql+diffs+tlo+thi

    gzFile     fp;    
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r"); 

    if (fp) {

        kstream_t *ks;
        ks  =  ks_init(fp);

        int qlo, qhi, ql;

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            
            fields = ksplit(&kt, '\t', &n);

            k = kh_put(strip, h, kt.s, &ret);
            if(ret == 1){
                STRIP_T *strip   = (STRIP_T *) malloc( sizeof(STRIP_T) );
                if( strip != NULL ){
                    strip->start   = -1;
                    strip->end     = -1;
                    strip->strand  = 1;
                    kh_key(h, k)   = strdup(kt.s);
                    kh_val(h, k)   = strip;
                }
            }

            qlo = atoi(kt.s + fields[3]);
            qhi = atoi(kt.s + fields[4]);
            ql  = atoi(kt.s + fields[5]);

            if((kt.s + fields[1])[0]  == '+' && (kt.s + fields[2])[0] == '+'){
                 if(qlo > width) continue;
                 kh_val(h, k)->start   = qhi;
                 kh_val(h, k)->strand  = 1;
            }else if((kt.s + fields[1])[0] == '-' && (kt.s + fields[2])[0] == '-'){
                 if( ql - qhi > width) continue;
                 kh_val(h, k)->end     = qlo - 1;
                 kh_val(h, k)->strand  = 1;
            }else if((kt.s + fields[1])[0] == '+' && (kt.s + fields[2])[0] == '-'){
                 if(qlo > width) continue;
                 kh_val(h, k)->start   = qhi ;
                 kh_val(h, k)->strand  = -1;
            }else if((kt.s + fields[1])[0] == '-' && (kt.s + fields[2])[0] == '+'){
                 if( ql - qhi  > width) continue;
                 kh_val(h, k)->end   = qlo - 1;;
                 kh_val(h, k)->strand  = -1;
            }

        }
        ks_destroy(ks);
        gzclose(fp);

    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind]);
        exit(1);
    }

    fp = strcmp(argv[optind + 1], "-")? gzopen(argv[optind + 1], "r") : gzdopen(fileno(stdin), "r");
    if( fp ){
        
        kseq_t *kseq;
        kseq = kseq_init(fp);

        kstring_t  kv  = {0, 0, 0};

        int l = 0;
        while ((l = kseq_read(kseq)) >= 0){
           
            k = kh_get(strip, h, kseq->name.s);
            if(k == kh_end(h) ) continue;        
            if(kh_val(h, k)->start == -1 && kh_val(h, k)->end == -1) continue;

            kt.l = kv.l = 0;

            if(kh_val(h, k)->start == -1) kh_val(h, k)->start = 0;
            if(kh_val(h, k)->end   == -1) kh_val(h, k)->end = kseq->seq.l;

            kseq->seq.s[kh_val(h, k)->end]  = '\0' ;    
            kputs(kseq->seq.s  + kh_val(h, k)->start, &kt);
            if( kh_val(h, k)->strand == -1) rev_com(&kt);

            if(kseq->qual.l != 0){
                 kseq->qual.s[kh_val(h, k)->end] = '\0' ;
                 kputs(kseq->qual.s + kh_val(h, k)->start, &kv);
                if( kh_val(h, k)->strand == -1) rev(&kv);
            }

            (kseq->qual.l != 0) ?
                        printf("@%s\n%s\n+\n%s\n", kseq->name.s, kt.s, kv.s) :
                        printf(">%s\n%s\n", kseq->name.s, kt.s);

        }

        kseq_destroy(kseq);
        gzclose(fp);
    
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind + 1]);
        exit(-1); 
    }

    kh_strip_destroy(h);

    return 0;
}
