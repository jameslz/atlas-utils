#include "utils.h"
#include "kvec.h"

static kvec_t( const char* ) vs;
static kvec_t(double) sum;

int counts2freqs_main(int argc, char *argv[]){
    
    if ( argc != 2) {
        
        fprintf(stderr, "\nUsage:  atlas-utils counts2freqs  <counts.table>\n\n");
        return 1;
    
    }

    kstring_t kt = {0, 0, 0};
    int *fields, n, i;

    kv_init(sum);
    kv_init(vs);

    gzFile fp;
    fp = strcmp(argv[1], "-")? gzopen(argv[1], "r") : gzdopen(fileno(stdin), "r");  
    if (fp) {

        kstream_t *ks;
        ks = ks_init(fp);

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){

            kv_push(const char*, vs, strdup(kt.s));
            fields = ksplit(&kt,'\t', &n);

            if(kt.s[0] == '#'){
                for (i = 1; i < n; ++i) kv_push(double, sum, 0 );
                continue;
            } else {
                for (i = 1; i < n; ++i) kv_a(double, sum, i - 1 ) +=  atof(kt.s + fields[i]);
            }
            
        }

        ks_destroy(ks);
        gzclose(fp);

    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[1]);
        exit(0);
    }

    kstring_t kv = {0, 0, 0};
    for (i = 0; i < kv_size(vs); ++i){

        kt.l = kv.l = 0;
        kputs( kv_A(vs, i), &kt);

        if(kt.s[0] == '#'){
            puts(kt.s); continue;
        }else{
            fields = ksplit(&kt, '\t', &n);
            kputs(kt.s, &kv);
            int j;
            for (j = 1; j < n; ++j){
                kputc('\t', &kv);
                ( atoi(kt.s + fields[j]) != 0 ) ?  
                     ksprintf(&kv, "%.4g",  atof(kt.s + fields[j]) / kv_A(sum, j - 1) ) :
                     kputc('0', &kv);

                
            }

            puts(kv.s);
        }
    }
    
    kv_destroy(sum);
    kv_destroy(vs);
    
    free(kt.s);
    free(kv.s);

    return 0;
}