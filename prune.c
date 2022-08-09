#include "utils.h"

int prune_main(int argc, char *argv[]){

    double t = .0;
    int c;
    while ((c = getopt(argc, argv, "t:")) >= 0) {
        if (c == 't') t = atof(optarg);
    }
    
    if ( optind == argc || argc != optind + 1) {

        fprintf(stderr, "\n\nUsage: atlas-utils prune [option] <otutab>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -t  threshold value for feature to trim.\n");
        fprintf(stderr, "      all values in the feature below the the threshold.\n");
        fprintf(stderr, "      default. [.0];\n\n");
        return 1;
    
    }  

    gzFile fp;
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r");
    if (fp) {

        kstream_t *ks;
        ks = ks_init(fp);        

        int *fields, n, i;

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

                for (i = 1; i < n; ++i){
                    if(atof(kt.s + fields[i]) > t ){
                       puts(kv.s);
                       break;
                    }
                }
                
            }

        }
        
        free(kt.s);
        free(kv.s);
        ks_destroy(ks);
        gzclose(fp);
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind]);
        exit(1);
    }

    return 0;

}