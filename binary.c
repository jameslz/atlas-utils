#include "utils.h"

int binary_main(int argc, char *argv[]){
    
    double t = .0;
    int c;
    while ((c = getopt(argc, argv, "t:")) >= 0) {
        if (c == 't') t = atof(optarg);
    }

    if ( optind == argc || argc != optind + 1) {

        fprintf(stderr, "\n\nUsage: atlas-utils binary [option] <Numeric Table|->\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -t  threshold value to set 1.; [default: 0]\n\n");
        return 1;
    
    }  

    kstring_t kt = {0, 0, 0};

    gzFile     fp;
    fp = strcmp(argv[ optind ], "-")? gzopen(argv[ optind ], "r") : gzdopen(fileno(stdin), "r");

    if (fp) {
           
        kstream_t *ks;
        ks = ks_init(fp);

        if(ks_getuntil(ks, '\n', &kt, 0) >=  0 && kt.l > 0){
            if(kt.s[0] == '#') puts(kt.s);
        }

        int *fileds, i, n;
        double val;

        while(ks_getuntil(ks, '\n', &kt, 0) >=  0){
            
            fileds =  ksplit(&kt, '\t', &n);
            fputs(kt.s, stdout);
            for (i = 1; i < n; ++i){
               val = atof(kt.s + fileds[i]);
               (val > t) ? fputs("\t1", stdout) : fputs("\t0", stdout);
            }
            fputc('\n', stdout);

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
