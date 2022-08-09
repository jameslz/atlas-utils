#include "utils.h"

int annotation_main(int argc, char *argv[]){
    
    if ( optind == argc || argc != optind + 2) {
        
        fprintf(stderr, "\nUsage: atlas-utils annotation <annotation> <otutab>\n\n");
        return 1;
    
    }

    kstring_t kt = {0, 0, 0};
    int *fields, n, ret;

    khash_t(reg) *h;
    h = kh_init(reg);
    khint_t k;

    gzFile     fp;    
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r"); 

    if (fp) {

        kstream_t *ks;
        ks  =  ks_init(fp);

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            
            fields = ksplit(&kt, '\t', &n);
            if(n != 2) continue;
            k = kh_put(reg, h, kt.s, &ret);
            kh_key(h, k)   = strdup(kt.s);
            kh_val(h, k)   = strdup(kt.s + fields[1]);
        }
        ks_destroy(ks);
        gzclose(fp);

    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind]);
        exit(1);
    }

    fp = strcmp(argv[optind + 1], "-")? gzopen(argv[optind + 1], "r") : gzdopen(fileno(stdin), "r"); 
    
    if (fp) {

        kstream_t *ks;
        ks           =  ks_init(fp);
        kstring_t kv =  {0, 0, 0};

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){

            if( kt.l == 0 ) continue;

            if( kt.s[0] == '#'){
               if( kt.s[1] == ' ') continue;
               printf("%s\ttaxonomy\n", kt.s); continue;
               continue;
            }

            kv.l = 0;
            kputs(kt.s, &kv);
            fields = ksplit(&kt, '\t', &n);
            k = kh_get(reg, h, kt.s);
            
            if(k != kh_end(h)){
                kputc('\t', &kv);
                kputs(kh_val(h, k), &kv);
            }else{
                kputc('\t', &kv);
                kputs("Unclassified", &kv);
            }
            
            puts(kv.s);
        }
        free(kv.s);
        ks_destroy(ks);
        gzclose(fp);
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind + 1]);
        exit(1);
    }

    kh_reg_destroy( h );
    free(kt.s);
    return 0;
}
