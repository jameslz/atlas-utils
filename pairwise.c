#include "utils.h"

int pairwise_main(int argc, char *argv[]){
    
    if ( optind == argc || argc > optind + 1) {
        fprintf(stderr, "\nUsage: atlas-utils pairwise <otutab>\n\n");
        return 1;
    }

    gzFile fp;
    fp = strcmp(argv[1], "-")? gzopen(argv[1], "r") : gzdopen(fileno(stdin), "r");
    if (fp) {

        kstream_t *ks;
        ks = ks_init(fp);
        int i, *fields, m, n;
        
        kstring_t  kt  = {0, 0, 0};
        kstring_t  kv  = {0, 0, 0};
        int *elem;

        if( ks_getuntil(ks, '\n', &kt, 0) ){
            
            if(kt.s[0] == '\t') kputc('#',  &kv);
            kputs(kt.s, &kv);
            elem = ksplit(&kv, '\t', &m); 
        
        }else{
            fprintf(stderr, "[ERR]: file empty? \n");
            return 0;
        }

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){

            fields = ksplit(&kt, '\t', &n);
            for (i = 1; i < n; ++i) printf("%s\t%.4g\t%s\n", kv.s + elem[ i ], atof(kt.s + fields[i]), kt.s);

        }


        free(kv.s);
        free(kt.s);
        ks_destroy(ks);
        gzclose(fp);
    
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[1]);
        exit(1);
    }

    return 0;
}