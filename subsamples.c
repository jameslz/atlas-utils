#include "utils.h"
#include "kvec.h"

static kvec_t(int) vt;

void subsamples_fileds(khash_t(int) *h, char *samples);

int subsamples_main(int argc, char *argv[]){
    
    if ( argc != 3) {
        fprintf(stderr, "\nUsage: atlas-utils subsamples <otu-table>  <samples| ',' >\n\n");
        return 1;
    }

    khash_t(int) *h;
    h = kh_init(int);

    gzFile fp;
    fp = strcmp(argv[1], "-")? gzopen(argv[1], "r") : gzdopen(fileno(stdin), "r");
    if (fp) {
        
        kstream_t *ks;
        ks = ks_init(fp);

        kstring_t  kt    = {0, 0, 0};
        int i, *fields, n, ret;
        int ann_field    = 0;
        khint_t k;

        kstring_t kv = {0, 0, 0};

        if( ks_getuntil( ks, '\n', &kt, 0) ){

            if(kt.s[0] == '#'){
                
                fields = ksplit(&kt, '\t', &n);
                
                if( strcmp(kt.s + fields[n - 1], "taxonomy") == 0) ann_field = 1;
                for (i = 1; i < n; ++i){
                    k = kh_put(int, h, kt.s + fields[i],  &ret);
                    kh_key(h, k) = strdup(kt.s + fields[i]);
                    kh_val(h, k) = i;
                }
                
                subsamples_fileds(h, argv[2]);

                if (kv_size(vt) == 0){
                   fprintf(stderr, "[ERR]: can't map sample ids in file: %s", argv[2]);
                   exit(-1);
                }

                kputs(kt.s , &kv);
                for (i = 0; i < kv_size(vt); ++i) ksprintf(&kv, "\t%s", kt.s + fields[ kv_A(vt, i) ]);
                if(ann_field == 1) ksprintf(&kv, "\t%s", kt.s + fields[n - 1]);  
                puts(kv.s);

            }else{
                fprintf(stderr, "[ERR]: no title line start with '#' ? \n");
                return 0;
            }

        }else{
            fprintf(stderr, "[ERR]: file empty? \n");
            return 0;
        }

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
        
            fields = ksplit(&kt, '\t', &n);

            kv.l = 0;
            kputs(kt.s , &kv);
            for (i = 0; i < kv_size(vt); ++i) ksprintf(&kv, "\t%s", kt.s + fields[ kv_A(vt, i) ]);
            if(ann_field == 1) ksprintf(&kv, "\t%s", kt.s + fields[n - 1]);  
            puts(kv.s);

        }

        free(kt.s);
        free(kv.s);
        ks_destroy(ks);
        gzclose(fp);
    
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[1]);
        exit(1);
    }

    kh_int_destroy(h);
    kv_destroy(vt);

    return 0;
}

void subsamples_fileds(khash_t(int) *h, char *samples){

        kstring_t  kt    = {0, 0, 0};
        kputs(samples, &kt);

        int *fields, n, i;
        fields = ksplit(&kt, ',' , &n);
        khint_t k;
        for (i = 0; i < n; ++i){  
	        k = kh_get(int, h, kt.s + fields[i]);
	        if(k != kh_end(h)) kv_push(int, vt, kh_val(h, k));
        }

        free(kt.s);
}