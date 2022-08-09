#include "utils.h"
KHASH_MAP_INIT_STR(normalization, int)

void kh_normalization_destroy(khash_t(normalization) *h);

int normalization_main(int argc, char *argv[]){

    if ( argc != 3) {
        fprintf(stderr, "\nUsage: atlas-utils normalization <copy_number> <otutab>\n\n");
        return 1;
    }

    khash_t(normalization) *h;
    h = kh_init(normalization);
    
    khint_t k;

    kstream_t  *ks;
    kstring_t   kt  = {0, 0, 0};

    gzFile     fp;
    fp = strcmp(argv[1], "-")? gzopen(argv[1], "r") : gzdopen(fileno(stdin), "r"); 
   
    if(fp){
        
        ks = ks_init(fp);
        int ret;
        char *p;
        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            if( kt.l == 0 ) continue;
            ks_tokaux_t aux;
            p = kstrtok(kt.s, "\t", &aux);
            kt.s[aux.p - p] = '\0';

            k = kh_put(normalization, h, p, &ret);
            if(ret){
                kh_key(h, k) = strdup(p);
                kh_val(h, k) = atoi(aux.p + 1);
            }
        }

        ks_destroy(ks);
        gzclose(fp);
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[1]);
        exit(1);
    }

    fp = strcmp(argv[2], "-")? gzopen(argv[2], "r") : gzdopen(fileno(stdin), "r");    
    if(fp){

        ks             = ks_init(fp);
        kstring_t  kv  = {0, 0, 0};
        int  i, *fields, n;
        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            
            if(kt.l == 0) continue;
            if(kt.s[0] == '#') {         
                puts(kt.s);
                continue;     
            }
            
            kv.l = 0;
            kputs(kt.s, &kv);

            fields = ksplit(&kt, '\t', &n);
            k = kh_get(normalization, h, kt.s);
            if(k == kh_end(h)){
                puts(kv.s);
                continue;
            }

            kv.l = 0;
            kputs(kt.s ,  &kv);
            int cnt = kh_val(h, k);
            for (i = 1; i < n; ++i) ksprintf(&kv, "\t%g",  atof(kt.s + fields[i])/cnt );

            puts(kv.s);

        }
        free(kv.s);
        ks_destroy(ks);
        gzclose(fp);
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[2]);
        exit(1);
    }
    
    free(kt.s);
    kh_normalization_destroy(h);
    return 0;
}

void kh_normalization_destroy(khash_t(normalization) *h){

    khint_t k;
    if (h == 0) return;
    for (k = 0; k < kh_end(h); ++k) {
        if (kh_exist(h, k)) {
            free((char*)kh_key(h, k));
        }
    }
    kh_destroy(normalization, h);
}
