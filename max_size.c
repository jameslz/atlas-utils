#include "utils.h"
#include "kvec.h"

static kvec_t(int) sum;
#define khint_lt(a, b) (a < b)
KSORT_INIT(max, int, khint_lt)

int max_size_main (int argc, char *argv[]){
    
    if ( argc != 2) {
        
        fprintf(stderr, "\nUsage: atlas-utils max_size <otutab>\n\n");
        return 1;
    
    }

    kstring_t kt = {0, 0, 0};
    int *fields, n, i;

    kv_init(sum);

    gzFile fp;
    fp = strcmp(argv[1], "-")? gzopen(argv[1], "r") : gzdopen(fileno(stdin), "r");
    if (fp) {

        kstream_t *ks;
        ks = ks_init(fp);

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){

            if(kt.s[0] == '#'){
               fields = ksplit(&kt,'\t', &n);
               for (i = 1; i < n; ++i) kv_push(int, sum, 0 );
               continue;
            }

            fields = ksplit(&kt,'\t', &n);
            for (i = 1; i < n; ++i) kv_a(int, sum, i - 1 ) +=  atof(kt.s + fields[i]);            
        
        }

    	free(kt.s);
        ks_destroy(ks);
        gzclose(fp);

    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[1]);
        exit(1);
    }

    ks_mergesort(max, sum.n, sum.a, 0);
    printf("%d\n", sum.a[sum.n - 1]);

    kv_destroy(sum);
    return 0;

}