#include "utils.h"

int main(int argc, char *argv[]){
    
    if ( optind == argc || argc != optind + 1) {
        
        fprintf(stderr, "\nUsage: atlas-utils kotu <otu>\n\n");
        return 1;
    
    }

    kstring_t kt = {0, 0, 0};
    int *fields, i, n;

    gzFile     fp;    
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r");
    
    int id = 1;
    if (fp) {

        kstream_t *ks;
        ks  =  ks_init(fp);

        if( ks_getuntil( ks, '\n', &kt, 0) > 0){
            
            fields = ksplit(&kt, '\t', &n);
            printf("#OTU ID");
            for (i = 1; i < n; ++i) printf("\t%s",   kt.s + fields[i]);
            printf("\n");

        }

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            
            fields = ksplit(&kt, '\t', &n);
        
            printf("KOTU_%d", id);
            for (i = 1; i < n; ++i) printf("\t%s",  kt.s + fields[i]);
            printf("\n");
            fprintf(stderr, "KOTU_%d\t%s\n", id, kt.s);
            
            ++id;

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
