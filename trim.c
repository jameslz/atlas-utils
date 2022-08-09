#include "utils.h"

int trim_main(int argc, char *argv[]){
    
    if ( optind == argc || argc != optind + 2) {

        fprintf(stderr, "\nUsage: atlas-utils trim <otutab> <cutoff:2>\n\n");
        return 1;
    }    

    gzFile fp;
    fp = strcmp(argv[1], "-")? gzopen(argv[1], "r") : gzdopen(fileno(stdin), "r");
    if (fp) {

        kstream_t *ks;
        ks = ks_init(fp);        

        int *fields, n, i;
        double cutoff, value;
        cutoff = atof(argv[2]);

        kstring_t kt    = {0, 0, 0};
        kstring_t kv    = {0, 0, 0};
        int ann_field   = 0;

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            
            if(kt.s[0] == '#'){
                puts(kt.s);
                if(kt.l < 8) continue;
                if(strcmp(kt.s + kt.l - 8, "taxonomy") == 0) ann_field = 1;
                continue;
            } else {
                
                kv.l = 0;
                kputs(kt.s, &kv);
                fields = ksplit(&kt, '\t', &n);
                
                n -= ann_field;
                for (value = 0, i = 1; i < n; ++i) value += atof(kt.s + fields[i]);
                if(value == 0.0 || value < cutoff) continue;
                puts(kv.s);

            }

        }
        
        free(kt.s);
        free(kv.s);
        ks_destroy(ks);
        gzclose(fp);
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[1]);
        exit(1);
    }

    return 0;

}