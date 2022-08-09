#include "utils.h"

int strip_main(int argc, char *argv[]){
    
    if ( optind == argc || argc != optind + 1) {
        
        fprintf(stderr, "\nUsage: atlas-utils strip <tsv>\n\n");
        return 1;
    
    }

    kstring_t kt = {0, 0, 0};
    int *fields, i, n;

    gzFile     fp;    
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r");
    
    if (fp) {

        kstream_t *ks;
        ks  =  ks_init(fp);

        if(ks_getuntil(ks, '\n', &kt, 0) >=  0){
             
             if(kt.l == 0){
                fprintf(stderr, "[ERR]: empty file? \n");
                exit(-1);
             }

             if(kt.s[0] == '#') puts(kt.s);
             else{
                fprintf(stderr, "[ERR]: headline not start with [#]\n");
                exit(-1);
             }
        }


        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            
            fields = ksplit(&kt, '\t', &n);
            for (i = 0; i < fields[1]; ++i)
                if(kt.s[i] == ' ') kt.s[i] = '\0';

            printf("%s",  kt.s);
            for (i = 1; i < n; ++i) printf("\t%s",  kt.s + fields[i]);
            printf("\n");

        }
        ks_destroy(ks);
        gzclose(fp);

    }else{
        
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind]);
        exit(1);
    
    }

    free(kt.s);
    return 0;
}
