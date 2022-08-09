#include "utils.h"

void rarity_print( kstring_t *kt,  khash_t(reg)  *h, const char *label);

int rarity_main(int argc, char *argv[]){
    
    int c;
    int norm = 1;

    while ((c = getopt(argc, argv, "n:")) >= 0) {
        if (c == 'n')  norm   = atoi(optarg);
    }


    if ( optind == argc || argc != optind + 1) {

        fprintf(stderr, "\nUsage: atlas-utils [option] rarity <tsv>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -n INT  normalization factor, ie: 100 default: [1]\n\n");
        return 1;
    
    }  


    kstring_t kt      = {0, 0, 0};
    kstring_t kv      = {0, 0, 0};

    kstring_t kt_rt   = {0, 0, 0};
    kstring_t kt_at   = {0, 0, 0};
    kstring_t kt_mt   = {0, 0, 0};
    kstring_t kt_crt  = {0, 0, 0};
    kstring_t kt_cat  = {0, 0, 0};
    kstring_t kt_crat = {0, 0, 0};

    gzFile     fp;
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r"); 

    khash_t(reg) *h;
    h = kh_init(reg);
    khint_t k;

    if (fp) {

        kstream_t *ks;
        ks  =  ks_init(fp);
        int l1, l2, l3, *fields, n, i, ret;

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            
            if(kt.s[0] == '#'){
                printf("%s\tType\n", kt.s);
                continue;
            }

            kv.l = 0;
            kputs(kt.s, &kv);

            fields = ksplit(&kt, '\t', &n);
            k = kh_put(reg, h, kt.s, &ret);
            kh_key(h, k) = strdup(kt.s);
            kh_val(h, k) = strdup(kv.s);
            
            l1 = l2 = l3 = 0;
            for (i = 1; i < n; ++i){
               
               double val = atof(kt.s + fields[i]) * norm;
               if(val > 1) l3++;
               else if(val > 0.1) l2++;
               else l1++;
            
            }

            if(l1 == n - 1){
                ksprintf(&kt_rt, "%s," ,kt.s);
                continue;
            }
            if(l3 == n - 1){
                ksprintf(&kt_at, "%s," ,kt.s);
                continue;
            }
            if(l2 == n - 1){
                ksprintf(&kt_mt, "%s," ,kt.s);
                continue;
            }

            if(l1 > 0 && l2 > 0 && l3 == 0){
                ksprintf(&kt_crt, "%s," ,kt.s);
                continue;
            }
            
            if(l1 == 0 && l2 > 0 && l3 > 0){
                ksprintf(&kt_cat, "%s," ,kt.s);
                continue;
            }           

            if(l1 > 0 && l3 > 0){
                ksprintf(&kt_crat, "%s," ,kt.s);
                continue;
            } 

        }
        ks_destroy(ks);
        gzclose(fp);

    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind]);
        exit(1);
    }

    if(kt_rt.l   > 0)  rarity_print(&kt_rt,   h, "RT");
    if(kt_at.l   > 0)  rarity_print(&kt_at,   h, "AT");
    if(kt_mt.l   > 0)  rarity_print(&kt_mt,   h, "MT");
    if(kt_cat.l  > 0)  rarity_print(&kt_cat,  h, "CAT");
    if(kt_crt.l  > 0)  rarity_print(&kt_crt,  h, "CRT");
    if(kt_crat.l > 0)  rarity_print(&kt_crat, h, "CRAT");

    kh_reg_destroy( h );

    free(kt.s);
    free(kv.s);
    
    free(kt_rt.s);
    free(kt_at.s);
    free(kt_mt.s);
    free(kt_cat.s);
    free(kt_crt.s);
    free(kt_crat.s);

    return 0;
}

void rarity_print(kstring_t *kt,  khash_t(reg)  *h, const char *label){

    int i, n, *fields;
    khint_t k;

    kt->s[kt->l] = '\0';
    fields =  ksplit(kt, ',',  &n);
    for (i = 0; i < n; ++i){
        k = kh_get(reg, h, kt->s + fields[i]);
        if(k != kh_end(h)) printf("%s\t%s\n", kh_val(h, k), label);
    }

}