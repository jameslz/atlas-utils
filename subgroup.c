#include "utils.h"
#include "kvec.h"

int subgroup_main(int argc, char *argv[]){
    
    int key   = 0;
    int c;
    while ((c = getopt(argc, argv, "k")) >= 0) {
        if(c =='k') key  = 1;
    }

    if ( optind == argc || argc != optind + 2) {
        fprintf(stderr, "\n\nUsage:atlas-utils subgroup [options] <tsv> <id>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -k       query using key id.\n\n");
        return 1;
    }

    khash_t(reg32) *h;
    h = kh_init(reg32);

    kstream_t *ks;
    kstring_t  kt    = {0, 0, 0};
    khint_t k;
    int ncols  = 0;
    int i, *fields, n, ret;

    gzFile fp;
    if(key == 1){        
        
        kputs(argv[optind + 1], &kt);
        fields = ksplit(&kt, ',', &n);
        for (i = 0; i < n; ++i){
            k = kh_put(reg32, h, kt.s + fields[i], &ret);
            if(ret == 1){
               kh_key(h, k) = strdup( kt.s + fields[i] );
            }else if(ret == 0){
               fprintf(stderr, "[WARN]: ID %s repeat, will ignor.\n", kt.s + fields[i]);
            }
        }

    }else{

        fp = strcmp(argv[optind + 1], "-")? gzopen(argv[optind + 1], "r") : gzdopen(fileno(stdin), "r");
        if (fp) {
            ks = ks_init(fp);            
            while( ks_getuntil( ks, '\n', &kt, 0) >=  0){            
                
                k = kh_put(reg32, h, kt.s, &ret);
                if(ret == 1){
                   kh_key(h, k) = strdup( kt.s );
                }else if(ret == 0){
                   fprintf(stderr, "[WARN]: ID %s repeat, will ignor.\n", kt.s);
                }
            
            }
            ks_destroy(ks);
            gzclose(fp);

        }else{
            fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind + 1]);
            exit(1);
        }

    }

    kvec_t(int) location;
    kv_init(location);

    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r");
    if (fp) {
        
        ks = ks_init(fp);
        kstring_t kv  = {0, 0, 0};

        if( ks_getuntil(ks, '\n', &kt, 0) ){

            fields = ksplit(&kt, '\t', &n);

            for (i = 0; i < n; ++i){
                k = kh_get(reg32, h, kt.s + fields[i]);
                if( k != kh_end(h) ){
                    ncols++;
                    kv_push(int, location, i);
                    ksprintf(&kv, "\t%s", kt.s + fields[i]);
                }
            }

            if(ncols == 0){
                fprintf(stderr, "[ERR]: no sample match in titles line.\n");
                return 0;
            }

            puts(kv.s + 1);

           
        }else{
            fprintf(stderr, "[ERR]: file empty? \n");
            return 0;
        }

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){

            fields = ksplit(&kt, '\t', &n);
            kv.l = 0;
            for (i = 0; i < ncols; ++i) ksprintf(&kv, "\t%s", kt.s + fields[ kv_A(location, i) ]);
            puts(kv.s + 1);

        }

        free(kv.s);
        ks_destroy(ks);
        gzclose(fp);

    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind]);
        exit(1);
    }

    free(kt.s);
    kh_reg32_destroy(h);


    return 0;
}