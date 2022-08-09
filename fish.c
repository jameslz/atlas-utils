#include "utils.h"

int fish_main(int argc, char *argv[]){

    int  c;
    double threshold = 0.05;

    while ((c = getopt(argc, argv, "t:")) >= 0) {
        if (c == 't')  threshold = atof(optarg);
    }

    if ( optind == argc || argc != optind + 1) {

        fprintf(stderr, "\nUsage: atlas-utils fish [options] <otu>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "   -t DOUBLE abudance threshold to select. default:[0.05]\n");
        return 1;
    }    

    gzFile fp;
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r");
    if (fp) {

        kstring_t kt    = {0, 0, 0};
        kstring_t kv    = {0, 0, 0};

        kstream_t *ks;
        ks = ks_init(fp);

       
        if( ks_getuntil(ks, '\n', &kt, 0) >=  0 ){

            if(kt.s[0] == '#'){
                puts(kt.s);
            }else{
                fprintf(stderr, "[ERR]: first line not start with '#' \n %s\n", kt.s);
                exit(-1);
            }
        }

        int *fields, i, n;
        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            
            kv.l = 0;
            kputs(kt.s, &kv);

            fields  = ksplit(&kt, '\t', &n);
            for (i = 1; i < n; ++i){
            
               if( atof( kt.s + fields[i] ) >= threshold){
                  puts(kv.s);
                  break;
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