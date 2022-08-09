#include "utils.h"
#include <math.h>

static  khash_t(kreg) *ko;
static  khash_t(kreg) *brite;

#include "kvec.h"
static int n_sample;
static kstring_t  lt = {0, 0, 0};

void kann_titles(kstring_t *kt);
void kann_update_abundance(khint_t k, double abundance[n_sample]);

int  kann_main(int argc, char *argv[]){
    
    int R = 0;
    int c;
    while ((c = getopt(argc, argv, "rv")) >= 0) {
        if (c == 'r') R = 1;
    }

    if ( optind == argc || argc != optind + 2) {
        fprintf(stderr, "\nUsage: atlas-utils kann [options] <feature-map|KO\\tModule> <KO matrix>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -r  round value to nearest intege;\n\n");
        return 1;
    }

    ko  = kh_init(kreg);
    khint_t k;

    kstream_t  *ks;
    kstring_t   kt         = {0, 0, 0};
    kstring_t   samples    = {0, 0, 0};
 
    gzFile      fp;
    fp = strcmp(argv[optind + 1], "-")? gzopen(argv[optind + 1], "r") : gzdopen(fileno(stdin), "r");   
    if(fp){
        
        ks = ks_init(fp);
        char *p;
        int ret;
        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            
            if( kt.s[0] == '#'){
               if( kt.s[1] == ' ') continue;
                kputs(kt.s, &samples);
                continue;
            }

            ks_tokaux_t aux;
            p = kstrtok(kt.s, "\t", &aux);
            kt.s[aux.p - p] = '\0';

            k = kh_put(kreg, ko, p, &ret);
            kh_key(ko, k) = strdup(p);

            kstring_t ktmp = {0, 0, 0};
            kputs(aux.p + 1, &ktmp);
            kh_val(ko, k) = ktmp;

        }

        ks_destroy(ks);
        gzclose(fp);
    
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind + 1]);
        exit(1);
    }

    brite  = kh_init(kreg);
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r");   
    if(fp){
        
        ks = ks_init(fp);
        char *p;
        int ret;
        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){

            ks_tokaux_t aux;
            p = kstrtok(kt.s, "\t", &aux);
            kt.s[aux.p - p] = '\0';

            k = kh_get(kreg, ko, p);
            if(k == kh_end(ko)) continue;

            k = kh_put(kreg, brite, aux.p + 1, &ret);

            if( ret == 1 ){

                kh_key(brite, k) = strdup(aux.p + 1);
                kstring_t ktmp = {0, 0, 0};
                kputs(p, &ktmp);
                kh_val(brite, k)  = ktmp;

            }else{
                kputc(',', &kh_val(brite, k));
                kputs(p,   &kh_val(brite, k));
            }

        }

        ks_destroy(ks);
        gzclose(fp);
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind]);
        exit(1);
    }

    kann_titles(&samples);
    
    int n_sample;
    ksplit(&samples,  '\t', &n_sample);
    n_sample -= 1;

    double abundance[n_sample];

    kstring_t kv = {0, 0, 0};
    int i;
    for (k = 0; k < kh_end(brite); ++k){

        if(!kh_exist(brite, k)) continue;
    
        memset(abundance, 0, sizeof(abundance[0]) * n_sample );
        kann_update_abundance(k, abundance);

        kv.l = 0;
        kputs(kh_key(brite, k), &kv);
        for (i = 0; i < n_sample; ++i){
            kputc('\t', &kv);
            (R) ?  ksprintf(&kv, "%d", (int)round( abundance[i]) ) :
                   ksprintf(&kv, "%g", abundance[i]);
            
        }
        puts(kv.s);

    }

    kh_kreg_destroy(ko);
    kh_kreg_destroy(brite);
   
    free(kt.s);
    return 0;

}

void kann_titles(kstring_t *kt) {

    ks_tokaux_t aux;
    kstrtok(kt->s, "\t", &aux);
    printf("#catalog\t%s\n", aux.p + 1);

}

void kann_update_abundance(khint_t k, double abundance[n_sample]){

    int *fileds, n, i, j, *samples, n_sample;
    
    fileds =  ksplit(&kh_val(brite, k), ',', &n);
    for (i = 0; i < n; ++i){
        khint_t t = kh_get(kreg, ko, kh_val(brite, k).s+fileds[i]);
        if(t == kh_end(ko)){
            fprintf(stderr, "[ERR]: %s no abundance values!\n", kh_val(brite, k).s + fileds[i]);
        }else{
            lt.l = 0;

            kputs(kh_val(ko, t).s,  &lt);    
            samples = ksplit(&lt, '\t', &n_sample);
            for (j = 0; j < n_sample; ++j) abundance[j] += atof(lt.s + samples[j]);
        
        }
    }

}
