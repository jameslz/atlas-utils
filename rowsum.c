#include "utils.h"

int rowsum_main (int argc, char *argv[]){
    
    if ( optind == argc || argc != optind + 1) {

        fprintf(stderr, "\nUsage: atlas-utils rowsum <otutab>\n\n");
        return 1;
    }    

    gzFile fp;
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r");
    if (fp) {

        kstream_t *ks;
        ks = ks_init(fp);        

        int *fields, n, i;
        double sum;

        kstring_t kt    = {0, 0, 0};
        printf("#otu\tvalue\n");
        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            
            if(kt.s[0] == '#') continue;
            else {
                
                fields = ksplit(&kt, '\t', &n);
                
                for (sum = 0, i = 1; i < n; ++i) sum += atof(kt.s + fields[i]);
                printf("%s\t%g\n", kt.s, sum);

            }

        }
        
        free(kt.s);
        ks_destroy(ks);
        gzclose(fp);
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind]);
        exit(1);
    }

    return 0;

}