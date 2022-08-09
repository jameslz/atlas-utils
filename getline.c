#include "utils.h"

int getline_main(int argc, char *argv[]){
    
    if ( optind == argc || argc != optind + 2) {

        fprintf(stderr, "\nUsage: atlas-utils getline <tsv> <sample>\n\n");
        return 1;
    
    }  


    kstring_t kt = {0, 0, 0};

    gzFile     fp;
    fp = strcmp(argv[ optind ], "-")? gzopen(argv[ optind ], "r") : gzdopen(fileno(stdin), "r");

    if (fp) {
           
        kstream_t *ks;
        ks = ks_init(fp);

        if(ks_getuntil(ks, '\n', &kt, 0) >=  0 && kt.l > 0) puts(kt.s);
        
        char *p;
        int  l=0;       
        while(ks_getuntil(ks, '\n', &kt, 0) >=  0){
            
            if(kt.s[0] == '#') continue;
            ks_tokaux_t aux;
            p = kstrtok(kt.s, "\t", &aux);
            l = aux.p - p;
            if( strncmp(kt.s,  argv[ optind + 1] , l) == 0) puts(kt.s);

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
