#include "utils.h"

int patch_main(int argc, char *argv[]){
    
    if ( optind == argc || argc != optind + 2) {
        fprintf(stderr, "\nUsage: atlas-utils patch <tsv> <label:p>\n\n");
        return 1;
    }

    kstring_t kt = {0, 0, 0};

    gzFile     fp;
    fp = strcmp(argv[ optind ], "-")? gzopen(argv[ optind ], "r") : gzdopen(fileno(stdin), "r");

    if (fp) {
           
        kstream_t *ks;
        ks = ks_init(fp);

        while(ks_getuntil(ks, '\n', &kt, 0) >=  0){
            
            if(kt.s[0] == '#'){
                printf("%s\n", kt.s);
                continue;
            }
            printf("%s:%s\n", argv[optind + 1], kt.s);
        
        }
        
        ks_destroy(ks);
        gzclose(fp);

    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[ optind  ]);
        exit(1);
    }

    free(kt.s);
    return 0;
}
