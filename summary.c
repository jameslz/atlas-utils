#include "utils.h"
#include "kvec.h"

static kvec_t(int) otu;
static kvec_t(int) sum;

int summary_main(int argc, char *argv[]){
    
    if ( argc != 2) {
        
        fprintf(stderr, "\nUsage:  atlas-utils summary <otutab>\n\n");
        return 1;
    
    }

    kstring_t kt = {0, 0, 0};
    int *fields, n, i;

    kv_init(otu);
    kv_init(sum);

    gzFile fp;
    fp = strcmp(argv[1], "-")? gzopen(argv[1], "r") : gzdopen(fileno(stdin), "r");
    if (fp) {

        kstream_t *ks;
        ks = ks_init(fp);

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){

            if(kt.s[0] == '#'){
               puts(kt.s); 
               fields = ksplit(&kt,'\t', &n);
               for (i = 1; i < n; ++i){
                    kv_push(int, otu, 0 );
                    kv_push(int, sum, 0 );
               }
               continue;
            }

            fields = ksplit(&kt,'\t', &n);
            for (i = 1; i < n; ++i){
                if( atoi(kt.s + fields[i]) > 0 ) kv_a(int, otu, i - 1 ) += 1;
                kv_a(int, sum, i - 1 ) +=  atof(kt.s + fields[i]);
            }
            
        }

        ks_destroy(ks);
        gzclose(fp);

    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[1]);
        exit(1);
    }
    
    printf("tag_num");
    for (i = 0; i < kv_size(sum); ++i) printf("\t%d", kv_A(sum, i));
    printf("\n");

    printf("otu_num");
    for (i = 0; i < kv_size(otu); ++i) printf("\t%d", kv_A(otu, i));
    printf("\n");
   
    kv_destroy(otu);
    kv_destroy(sum);
    
    free(kt.s);

    return 0;
}