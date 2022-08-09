#include "utils.h"

int view_main(int argc, char *argv[]){
    
    int c;
    int comment = 0;
    int blank   = 0;

    while ((c = getopt(argc, argv, "cb")) >= 0) {
        if (c == 'c')  comment   = 1;
        else if (c == 'b') blank = 1;
    }

    if ( optind == argc || argc != optind + 1) {

        fprintf(stderr, "\nUsage: atlas-utils view <tsv>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "   -c   Add comment char '#' to the first line. \n");
        fprintf(stderr, "   -b   Remove the blank lines.\n\n");
        return 1;
    
    }  


    kstring_t kt = {0, 0, 0};

    gzFile     fp;
    fp = strcmp(argv[ optind ], "-")? gzopen(argv[ optind ], "r") : gzdopen(fileno(stdin), "r");

    if (fp) {
           
        kstream_t *ks;
        ks = ks_init(fp);

        if(ks_getuntil(ks, '\n', &kt, 0) >=  0 && kt.l > 0){
            if(comment) printf("%c", '#');
            puts(kt.s);
        }

        while(ks_getuntil(ks, '\n', &kt, 0) >=  0){
            
            if(kt.s[0] == '#') continue;
            if(blank && kt.l == 0) continue;
            puts(kt.s);
        
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
