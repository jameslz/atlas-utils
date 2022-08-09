#include "utils.h"
#include <math.h>
#include "kvec.h"

static int rnd  = 0;
static int ncol = 0;

KHASH_MAP_INIT_STR(hierarchy, double *)

void kh_hierarchy_destroy(khash_t(hierarchy) *h){
    
    khint_t k;
    if (h == 0) return;
    for (k = 0; k < kh_end(h); ++k) {
        if (kh_exist(h, k)) {
            free((char*)   kh_key(h, k));
            free((double*) kh_val(h, k));
        }
    }
    kh_destroy(hierarchy, h);
}

void hierarchy_print (int n, double *dv){
    int i;
    
    if(rnd){
       for (i = 0; i < n; ++i) printf("\t%d", (int)round( dv[i] ) );
    }else{
       for (i = 0; i < n; ++i) printf("\t%.4g", dv[i]);
    }
    
    putchar('\n');
}


int hierarchy_main (int argc, char *argv[]){

    int c;
    while ((c = getopt(argc, argv, "r")) >= 0) {
        if (c == 'r')  rnd   = 1;
    }

    if ( optind == argc || argc < optind + 1) {
        fprintf(stderr, "\n\nUsage: atlas-utils hierarchy [options] [otutab]\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "   -r   Round value to Int.\n\n");
        exit(1);
    }

    khash_t(hierarchy) *h;
    h    = kh_init(hierarchy);

    kstring_t kt     = {0, 0, 0};
    kstring_t head   = {0, 0, 0};
    kstring_t aux    = {0, 0, 0};
    kstring_t level  = {0, 0, 0};

    khint_t  k, t;

    gzFile fp;
    int i, j;

    kvec_t( const char* ) vs;
    kv_init(vs);


    fp = strcmp(argv[ optind ], "-")? gzopen(argv[ optind ], "r") : gzdopen(fileno(stdin), "r");
    if (fp) {

        kstream_t *ks;
        ks = ks_init(fp);

        int  *fields,  n, ret;
        int  *level_t, m;

        if(ks_getuntil( ks, '\n', &kt, 0) >=  0){

            if(ncol == 0){
               fields = ksplit(&kt, '\t', &n);
               kputs("#level", &head);
               for (i = 1; i < n - 1; ++i){
                    kputc('\t', &head);
                    kputs(kt.s + fields[i], &head);
               }
               ncol = n;
            }

        }else{
            fprintf(stderr, "[ERR]: Empty file?, %s ...\n" , argv[ optind ]);
            exit(-1);
        }


        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            
            fields  = ksplit(&kt, '\t', &n);

            if( n != ncol ){
                fprintf(stderr, "[ERR]: Please provide fields vith same size of headline, %s ...\n" , argv[ optind ]);
                exit(-1);
            }

            level.l = aux.l   = 0;
            kputs(kt.s + fields[n - 1], &level);

            level_t = ksplit(&level, ',', &m);
            for (i = 0; i < m; ++i){
                
                kputc('|',  &aux);
                kputs(level.s + level_t[i],  &aux);

                k = kh_get(hierarchy, h, aux.s);
                if( k == kh_end(h) ){
                    
                    double *dv    = (double *) malloc( (n - 1) * sizeof(double));
                    for (j = 1; j < n - 1; ++j) dv[ j - 1 ] = atof(kt.s + fields[j]);

                    t = kh_put(hierarchy, h, aux.s, &ret);
                    kh_key(h, t) = strdup(aux.s);
                    kh_val(h, t) = dv;
                    kv_push( const char *, vs, kh_key(h, t) );

                }else{
                    for (j = 1; j < n; ++j)  kh_val(h, k)[ j - 1 ] += atof(kt.s + fields[j]);
                }

            }
         
         }
         
         ks_destroy(ks);

    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[ optind ]);
        exit(1);
    }

    gzclose(fp);


    if(kh_size(h) > 0){
        
        printf("%s\n", head.s);
        for (i = 0; i < kv_size(vs); ++i) {
             khint_t k;
             k = kh_get(hierarchy, h, kv_A(vs, i));
             if (kh_exist(h, k)) {
                printf("%s", kh_key(h, k) + 1);
                hierarchy_print(ncol - 2,  kh_val(h, k));
             }
        }

    }
    
    kh_hierarchy_destroy(h);
    free(kt.s);
    free(head.s);
    kv_destroy(vs);

    return 0;
}
