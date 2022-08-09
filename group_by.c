#include "utils.h"
#include "kvec.h"
#include <math.h>

static kvec_t( const char* ) vs;

typedef struct  {
    int     n;
    int     m;
    double  *fields;
} shape_t ;

KHASH_MAP_INIT_STR(shape, shape_t)

void kh_shape_destroy(khash_t(shape) *h){
    
    khint_t k;
    if  (h == 0) return;
    for (k = 0; k < kh_end(h); ++k) {
        if (kh_exist(h, k)) free((char*)kh_key(h, k));
    }
    kh_destroy(shape, h);

}

int group_by_main(int argc, char *argv[]){
 
    int R = 0, opt = 1;
    int c;
    while ((c = getopt(argc, argv, "rt:")) >= 0) {
        if (c == 't')      opt = atoi(optarg);
        else if (c == 'r') R = 1;

    }

    if ( optind == argc || argc != optind + 2) {
        fprintf(stderr, "\nUsage: atlas-utils group_by <tsv> <map>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -t  STR  operattion. 1:sum, 2: average. 3:cov\n");
        fprintf(stderr, "                       default: [1];\n");
        fprintf(stderr, "  -r  round value to nearest intege;\n\n");
        return 1;
    }

    khash_t(reg) *h;
    h = kh_init(reg);

    khash_t(shape) *melt;
    melt = kh_init(shape);

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
     
            k = kh_put(shape, melt, kt.s + fields[1], &ret);
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
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[ optind + 1 ]);
        exit(1);
    }

    for (i = 0; i < kv_size(vs); ++i){
         k = kh_get(shape, melt, kv_A(vs, i));
         if(k != kh_end( melt )) kh_val(melt, k).fields = (double*)calloc(kh_val(melt, k).n, sizeof(double));
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
                        t = kh_get(shape, melt, kh_val(h, k));                        
                        kh_val(melt, t).fields[kh_val(melt, t).m] = i;
                        ++kh_val(melt, t).m;
                    }
                }

                printf("%s", kt.s);
                for (i = 0; i < kv_size(vs); ++i){
                     k = kh_get(shape, melt, kv_A(vs, i));
                     if(kh_val(melt, k).m) printf("\t%s", kv_A(vs, i));
                }
                printf("\n");
            
            }else{
                fprintf(stderr, "[ERR]: no headline ?\n");
                return -1;
            }

        }else{
            fprintf(stderr, "[ERR]: file empty? \n");
            return -1;
        }
  
        double val = 0.0;

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){

            fields = ksplit(&kt, '\t', &n);

            printf("%s", kt.s);
            for (i = 0; i < kv_size(vs); ++i){
                 k = kh_get(shape, melt, kv_A(vs, i));
                 
                 int z = 0;
                 if(k != kh_end( melt )){
                     
                     val = 0.0;
                     int j;
                     for (j = 0; j < kh_val(melt, k).m; ++j){
                        int    q = kh_val(melt, k).fields[j];
                        double v = atof( kt.s + fields[q] ) ;
                        val += v;
                        if( v != 0. ) ++z;
                     }
                     
                     if(opt == 2) val = val/kh_val(melt, k).m;
                     else if(opt == 3) val = 1.0 * z/kh_val(melt, k).m;

                 }

                if(kh_val(melt, k).m){
                	(R) ? printf("\t%d", (int)round(val)) : printf("\t%.4g", val);
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
    kh_shape_destroy(melt);
    kv_destroy(vs);

    return 0;

}