#include "utils.h"
#include "kvec.h"
#include <math.h>

static kvec_t( const char* ) vs;

typedef struct  {
    int  n;
    int  m;
    int  *fields;
} shape_t ;

KHASH_MAP_INIT_STR(core, shape_t)

void kh_core_destroy(khash_t(core) *h){
    
    khint_t k;
    if  (h == 0) return;
    for (k = 0; k < kh_end(h); ++k) {
        if (kh_exist(h, k)) free((char*)kh_key(h, k));
    }
    kh_destroy(core, h);

}

int core_main (int argc, char *argv[]){
 
    double threshold = 0;
    int c;
    while ((c = getopt(argc, argv, "t:")) >= 0) {
        if (c == 't') threshold = atof(optarg);
    }

    if ( optind == argc || argc != optind + 2) {
        fprintf(stderr, "\n\nUsage: atlas-utils core [option] <tsv> <map>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -t  threshold value for exist or absence;\n\n");
        return 1;
    }

    khash_t(reg) *h;
    h = kh_init(reg);

    khash_t(core) *melt;
    melt = kh_init(core);

    khint_t k;
    int i, *fields, n, ret;
    kstream_t *ks;
    kstring_t  kt  = {0, 0, 0};

    gzFile fp;
    fp = strcmp(argv[ optind + 1 ], "-")? gzopen(argv[ optind + 1 ], "r") : gzdopen(fileno(stdin), "r");
    if (fp) {

        ks     = ks_init(fp);
        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){

            if(kt.s[0] == '#') continue;
            
            fields = ksplit(&kt, '\t', &n);    
            k = kh_put(reg, h, kt.s, &ret);
            kh_key(h, k) = strdup(kt.s);
            kh_val(h, k) = strdup(kt.s + fields[1]);
     
            k = kh_put(core, melt, kt.s + fields[1], &ret);
            if(ret == 1){
                kh_key(melt, k) = strdup(kt.s + fields[1]);
                kv_push( const char *, vs, kh_key(melt, k) );
                shape_t shape  = {0,  0,  0};
                kh_val(melt, k) = shape;
            }
            ++kh_val(melt, k).n;

        }
        ks_destroy(ks);
        gzclose(fp);

    }else{
        fprintf(stderr, "[ERR]：can't open file %s\n", argv[ optind + 1 ]);
        exit(1);
    }

    for (i = 0; i < kv_size(vs); ++i){
         k = kh_get(core, melt, kv_A(vs, i));
         if(k != kh_end( melt )) kh_val(melt, k).fields = (int*)calloc(kh_val(melt, k).n, sizeof(int));
    }

    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r");
    if (fp) {

        khint_t t;
        ks = ks_init(fp);
        if( ks_getuntil(ks, '\n', &kt, 0) > 0 ){
            
            if(kt.s[0] == '#'){
                
                fields = ksplit(&kt, '\t', &n);
                for (i = 0; i < n; ++i){
                    k = kh_get(reg, h, kt.s + fields[i]);
                    if( k != kh_end(h) ){
                        t = kh_get(core, melt, kh_val(h, k));                        
                        kh_val(melt, t).fields[kh_val(melt, t).m] = i;
                        ++kh_val(melt, t).m;
                    }
                }

                printf("#ZOTU");
                for (i = 0; i < kv_size(vs); ++i){
                     k = kh_get(core, melt, kv_A(vs, i));
                     if(kh_val(melt, k).m) printf("\t%s", kv_A(vs, i));
                }
                printf("\n");
            
            }else{
                fprintf(stderr, "[ERR]: No headline.\n");
                return -1;
            }

        }else{
            fprintf(stderr, "[ERR]: file empty? \n");
            return -1;
        }
  
        int val = 0;
        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){

            fields = ksplit(&kt, '\t', &n);

            printf("%s", kt.s);
            for (i = 0; i < kv_size(vs); ++i){
                 k = kh_get(core, melt, kv_A(vs, i));
                 if(k != kh_end( melt )){

                     val = 0;
                     int j;
                     for (j = 0; j < kh_val(melt, k).m; ++j){
                        int q = kh_val(melt, k).fields[j];
                        if(atof( kt.s + fields[q] ) > threshold )  ++val;
                     }
                 
                 }

                if(kh_val(melt, k).m){
                    printf("\t%.2lf",  100 * (double)val / kh_val(melt, k).m);
                }
            }
            printf("\n");

        }

        ks_destroy(ks);
        gzclose(fp);
    
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind]);
        exit(1);
    }

    free(kt.s);
    kh_reg_destroy(h);
    kh_core_destroy(melt);
    kv_destroy(vs);

    return 0;

}