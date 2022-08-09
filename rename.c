#include "utils.h"

int main(int argc, char *argv[]){

    int   MAPPING  = 0;
    char *label    = "OTU";

    int c;
    while ((c = getopt(argc, argv, "l:m")) >= 0) {
        if (c == 'm')  MAPPING = 1;
        else if (c == 'l')  label = optarg;
    } 

    if ( optind == argc || argc != optind + 1) {
        fprintf(stderr, "\nUsage: atlas-utils rename <tsv>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "   -l  STR label prefix for rename. default [OTU]\n");
        fprintf(stderr, "   -m      print mapping file.\n\n");
        return 1;
    }  


    kstring_t kt = {0, 0, 0};

    gzFile     fp;
    fp = strcmp(argv[ optind ], "-")? gzopen(argv[ optind ], "r") : gzdopen(fileno(stdin), "r");

    if (fp) {
           
        kstream_t *ks;
        ks = ks_init(fp);
        int *fields, n, i;
        int id = 1;

        if(ks_getuntil(ks, '\n', &kt, 0) >=  0){
            if(kt.s[0] == '#'){
                fields = ksplit(&kt, '\t', &n);
                printf("#OTU ID");
                for (i = 1; i < n; ++i) printf("\t%s", kt.s + fields[i]);
                printf("\n");
           }else{
                fprintf(stderr, "[ERR]: can't find headline start with #\n");
                exit(-1);
           }
            
        }

        while(ks_getuntil(ks, '\n', &kt, 0) >=  0){
            
            fields = ksplit(&kt, '\t', &n);
            printf("%s_%d", label, id);
            for (i = 1; i < n; ++i) printf("\t%s", kt.s + fields[i]);
            printf("\n");

            if(MAPPING) fprintf(stderr, "%s\t%s_%d\n", kt.s, label, id);

            id++;
        
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
