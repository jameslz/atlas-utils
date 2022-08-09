#include "utils.h"
#include "kvec.h"

int contrib_main(int argc, char *argv[]){

    int f = 1;
    int c;
    while ((c = getopt(argc, argv, "k:")) >= 0) {
        if (c == 'k') f = atoi(optarg);
    }
    
    if ( optind == argc || argc != optind + 2) {       
    
        fprintf(stderr, "\nUsage:  atlas-utils contrib [option] <mapping> <abundance>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -k INT  column as key. default: [1];\n\n");

        fprintf(stderr, "Note:\n");
        fprintf(stderr, "Need three columns, ZOTU|KO|copy\n\n");
        return 1;
    
    }

    int *fields, i, absent, n;
    kstring_t  kt  = {0, 0, 0};
    kstring_t  kv  = {0, 0, 0};
  
    khash_t(reg) *h;
    h = kh_init(reg);
    khint_t k;

    gzFile     fp; 
    fp = strcmp(argv[ optind + 1 ], "-")? gzopen(argv[optind + 1], "r") : gzdopen(fileno(stdin), "r"); 
    if( fp ){
        
        kstream_t *ks;
        ks = ks_init(fp);
        char *p;
        int l;

        if( ks_getuntil(ks, '\n', &kt, 0) >=  0 ){
            if(kt.s[0] == '#'){
                ks_tokaux_t aux;
                p = kstrtok(kt.s, "\t", &aux);
                l = aux.p - p;
                kt.s[l] = '\0';
                kputs(aux.p + 1, &kv);

            }else{
                fprintf(stderr, "[ERR]: first line not start with '#' \n %s\n", kt.s);
                exit(-1);
            }
        }

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){

            if( kt.l == 0 ) continue;

            ks_tokaux_t aux;
            p = kstrtok(kt.s, "\t", &aux);
            l = aux.p - p;
            kt.s[l] = '\0';

            k = kh_put(reg, h, kt.s, &absent);
            if(absent == 1){
                kh_key(h, k) = strdup(kt.s);
                kh_val(h, k) = strdup(aux.p + 1);
            }

        }
        
        ks_destroy(ks);
        gzclose(fp);
    
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind + 1]);
        exit(1);
    }

    fp = strcmp(argv[ optind ], "-")? gzopen(argv[ optind ], "r") : gzdopen(fileno(stdin), "r"); 
    if( fp ){
        
        kstream_t *ks;
        ks = ks_init(fp);
        double val = 0.;

        if( ks_getuntil(ks, '\n', &kt, 0) >=  0 ){
            if(kt.s[0] == '#'){
                
                fields = ksplit(&kt, '\t', &n);
                if(n != 3){
                    fprintf(stderr, "[ERR]: need tree columns data frame; \n");
                    exit(-1);
                }
                printf("%s\t%s\t%s\n", kt.s, kt.s + fields[1], kv.s);

            }else{
                fprintf(stderr, "[ERR]: first line not start with '#' \n %s\n", kt.s);
                exit(-1);
            }
        }

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){

            if( kt.l == 0 ) continue;
            
            fields = ksplit(&kt, '\t', &n);
            val = atof(kt.s + fields[2]);
            k = kh_get(reg, h, kt.s + fields[ f - 1 ]);

            if(k != kh_end(h) ){

                printf("%s\t%s", kt.s, kt.s + fields[1]);  
                kv.l = 0;
                kputs(kh_val(h, k), &kv);
                fields = ksplit(&kv, '\t', &n);
                for (i = 0; i < n; ++i) printf("\t%lf", val * atof(kv.s + fields[i]));
                printf("\n");
            
            }

        }
        
        ks_destroy(ks);
        gzclose(fp);
    
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind + 1]);
        exit(1);
    }


    free(kt.s);    
    free(kv.s);    
    kh_reg_destroy(h);
    return 0;
}