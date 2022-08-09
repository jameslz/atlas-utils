#include "utils.h"
#include "kvec.h"

static int num;
static double *dv;

void agg_dv_zero (int n, double *dv){
    int i;
    for (i = 0; i < n; ++i) dv[i] = 0.0;
}

int aggregate_main(int argc, char *argv[]){

    int d = 1;
    
    int c;
    while ((c = getopt(argc, argv, "k:")) >= 0) {
        if (c == 'k') d = atoi(optarg);
    }

    if ( optind == argc || argc != optind + 1) {       
    
        fprintf(stderr, "\nUsage: atlas-utils aggregate [optind] <tsv>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -k INT  First |INT| number of fields as main keys. default: [1];\n\n");
        return 1;
    
    }

    int *fields, i, absent, n;
    kstring_t  kt  = {0, 0, 0};

    kvec_t( const char* ) vs;
    kv_init(vs);
  
    khash_t(kreg) *h;
    h = kh_init(kreg);
    khint_t k;

    gzFile     fp; 
    fp = strcmp(argv[ optind ], "-")? gzopen(argv[optind ], "r") : gzdopen(fileno(stdin), "r"); 
    if( fp ){
        
        kstream_t *ks;
        ks = ks_init(fp);

        if( ks_getuntil(ks, '\n', &kt, 0) >=  0 ){
            if(kt.s[0] == '#'){
                puts(kt.s);
                fields  = ksplit(&kt, '\t', &num);
                num -= d;
                if(num <= 0){
                    fprintf(stderr, "[ERR]: NO enough data fields.\n");
                    exit(-1);
                }
            }else{
                fprintf(stderr, "[ERR]: first line not start with '#' \n %s\n", kt.s);
                exit(-1);
            }
        }

        int point = 0;
        int p     = 0;
        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            
            point = d;
            if( kt.l == 0 ) continue;
            for (i = 0; i < kt.l; ++i){
                if(kt.s[i] == '\t'){
                    --point; p = i;
                }
                if(point == 0) break;
            }
            kt.s[p] = '\0';

            k = kh_put(kreg, h, kt.s, &absent);
            if(absent == 1){
                kstring_t kd = {0, 0, 0};
                kputs(kt.s + p + 1, &kd);
                kh_key(h, k) = strdup(kt.s);
                kv_push( const char *, vs, kh_key(h, k) );
                kh_val(h, k) = kd;

            }else{
                kputc('\t', &kh_val(h, k));
                kputs(kt.s + p + 1, &kh_val(h, k));
            }

        }
        
        ks_destroy(ks);
        gzclose(fp);
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[ optind ]);
        exit(1);
    }

    dv = (double *) calloc(num, sizeof(double));
    int j,z;

    for (i = 0; i < kv_size(vs); ++i) {
        
        k = kh_get(kreg, h, kv_A(vs, i));
        if( k != kh_end(h) ){
            
            fields = ksplit( &kh_val(h, k), '\t',  &n);
            if(n == num){
                printf("%s", kh_key(h, k));
                for (z = 0; z < num; ++z) printf("\t%lf", atof(kh_val(h, k).s + fields[z]));
                printf("\n");
            }else{
                
                agg_dv_zero(num, dv);
                int size = n/num;
                for (j = 0; j < num; ++j){
                   for (z = 0; z < size; ++z){
                       dv[j] += atof( kh_val(h, k).s + fields[j + z * num]);
                   }
                }

                printf("%s", kh_key(h, k));
                for (z = 0; z < num; ++z) printf("\t%lf", dv[z]);
                printf("\n");
            }
        }
    }    

    free(kt.s);    
    kv_destroy(vs);
    kh_kreg_destroy(h);

    return 0;
}