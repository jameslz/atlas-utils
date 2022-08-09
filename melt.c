#include "utils.h"
#include <math.h>

typedef struct{
    kstring_t  otu;
    kstring_t  org;
} Node;

KHASH_MAP_INIT_STR(melt, Node)

void melt_titles(kstring_t *kt);
void kh_melt_destroy(khash_t(melt) *h);

#include "kvec.h"
static int n_feature;
static int R = 0;

void melt_normalize(khash_t(melt) *h, double **abundance);
void melt_print_mx(double **abundance, int n_sample, int n_feature, kstring_t  *kt, int *samples_t);

int melt_main(int argc, char *argv[]){
    
    int c;
    while ((c = getopt(argc, argv, "r")) >= 0) {
        if (c == 'r') R = 1;
    }

    if ( optind == argc || argc != optind + 2) {
        fprintf(stderr, "\nUsage: atlas-utils melt [options] <feature-matrix> <otutab>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -r  round value to nearest intege;\n\n");
        return 1;
    }

    khash_t(melt) *h;
    h = kh_init(melt);
    
    khint_t k;

    kstream_t  *ks;
    kstring_t   kt        = {0, 0, 0};
    kstring_t   samples   = {0, 0, 0};

    gzFile      fp;
    fp = strcmp(argv[optind + 1], "-")? gzopen(argv[ optind + 1], "r") : gzdopen(fileno(stdin), "r");   
    if(fp){
        
        ks = ks_init(fp);
        char *p;
        int ret;
        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            
            if( kt.l == 0 ) continue;
            if( kt.s[0] == '#'){
               if( kt.s[1] == ' ') continue;
                kputs(kt.s, &samples);
                continue;
            }

            ks_tokaux_t aux;
            p = kstrtok(kt.s, "\t", &aux);
            kt.s[aux.p - p] = '\0';

            Node node;

            k = kh_put(melt, h, p, &ret);
            kh_key(h, k) = strdup(p);

            kstring_t k_otu = {0, 0, 0};
            kstring_t k_org = {0, 0, 0};

            kputs(aux.p + 1, &k_otu);
            
            node.otu = k_otu;
            node.org = k_org;

            kh_val(h, k) = node;

        }

        ks_destroy(ks);
        gzclose(fp);
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind + 1]);
        exit(1);
    }

    kstring_t   features  = {0, 0, 0};
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r");   
    if(fp){
        
        ks = ks_init(fp);
        char *p;

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            
            if( kt.l == 0) continue;
            if(kt.s[0] == '#'){
                kputs(kt.s, &features);
                continue;
            }

            ks_tokaux_t aux;
            p = kstrtok(kt.s, "\t", &aux);
            kt.s[aux.p - p] = '\0';

            k = kh_get(melt, h, p);
            if(k == kh_end(h)) continue;

            kputs(aux.p + 1, &kh_val(h, k).org);

        }

        ks_destroy(ks);
        gzclose(fp);
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind]);
        exit(1);
    }

    melt_titles(&samples);

    int *features_t, n_sample;
    ksplit(&samples,  '\t', &n_sample);
    features_t = ksplit(&features, '\t', &n_feature);
    n_feature-=1;  n_sample -= 1;

    double **abundance;
    abundance  = (double **)calloc(n_sample, sizeof(double*));
    int i;
    for (i = 0; i < n_sample; ++i) abundance[i] = (double *)calloc(n_feature, sizeof(double));

    melt_normalize(h, abundance);
    melt_print_mx(abundance, n_sample, n_feature, &features, features_t);
    
    kh_melt_destroy(h);
    free(kt.s);
  
    for (i = 0; i < n_sample; ++i) free(abundance[i]);
    free(abundance);
    return 0;

}

void melt_print_mx(double **abundance, int n_sample, int n_feature, kstring_t *kt, int *features_t){

    int i, j;
    kstring_t kv = {0, 0, 0};
    for (i = 0; i < n_feature; ++i){
        kv.l = 0;
        kputs(kt->s + features_t[i + 1], &kv);
        for (j = 0; j < n_sample; ++j){
            kputc('\t', &kv);
            (R) ?  ksprintf(&kv, "%d", (int)round( abundance[j][i] ) ) :
                   ksprintf(&kv, "%g",abundance[j][i]);

        }
        puts(kv.s);
    }
    free(kv.s);
}

void melt_normalize(khash_t(melt) *h, double **abundance){

    kvec_t(double) vt;
    kv_init(vt);

    int i;
    for (i = 0; i < n_feature; ++i) kv_push(double, vt, 0.0);

    int *filelds, n, n_sample, j;
    khint_t k;
    for (k = 0; k < kh_end(h); ++k) {
        if (kh_exist(h, k))  {
          
          if(kh_val(h, k).org.l == 0) continue;
          filelds=ksplit(&kh_val(h, k).org, '\t', &n);
          for (i = 0; i < n; ++i) kv_A(vt, i) = atof(kh_val(h, k).org.s + filelds[i]);
            
          filelds=ksplit(&kh_val(h, k).otu, '\t', &n_sample);
          for (i = 0; i < n_sample; ++i){
                double attr = atof(kh_val(h, k).otu.s + filelds[i]);
                for (j = 0; j < n_feature; ++j) abundance[i][j] += attr * kv_A(vt, j);
          }

        }
    }
    kv_destroy(vt);
}

void melt_titles(kstring_t *kt) {

    ks_tokaux_t aux;
    kstrtok(kt->s, "\t", &aux);
    printf("#sample\t%s\n", aux.p + 1);

}

void kh_melt_destroy(khash_t(melt) *h){
    
    khint_t k;
    if  (h == 0) return;
    for (k = 0; k < kh_end(h); ++k) {
        if (kh_exist(h, k))  {

            free((char*)kh_key(h, k));
            free((char*)kh_val(h, k).otu.s);
            free((char*)kh_val(h, k).org.s);
        
        }
    }
    kh_destroy(melt, h);
}