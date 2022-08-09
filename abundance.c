#include "utils.h"

static khash_t(char) *map;

void kh_abund_init();
void abund_assign_value(kstring_t *kt, kstring_t *assign, char level);
void abund_print_abundance (khash_t(double) *h);

int abundance_main(int argc, char *argv[]){
    
    char level = 'g';
    int c;
    while ((c = getopt(argc, argv, "l:")) >= 0) {
        if (c == 'l') level = optarg[0];
    }

    if ( optind == argc || argc != optind + 2) {

        fprintf(stderr, "\nUsage: atlas-utils abundance [options] <otutab:annotated> <sample>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -l  taxonomy level in [dkcofgs] Default:['g']\n\n");
        return 1;
    }

    khash_t(double) *h;
    h = kh_init(double);
    khint_t k, t;

    map = kh_init(char);
    kh_abund_init();
    
    gzFile fp;
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r");
    if (fp) {

        kstream_t *ks;
        ks = ks_init(fp);

        kstring_t kt = {0, 0, 0};
        int col      = 0;

        if( ks_getuntil( ks, '\n', &kt, 0) ){

            if(kt.s[0] == '#'){
                col = get_target_column(&kt, argv[optind + 1]);
            }else{
                fprintf(stderr, "[ERR]: no titlt line start with '#' ? \n");
                return 0;
            }

        }else{
            fprintf(stderr, "[ERR]: file empty? \n");
            return 0;
        }
        

        int  *fields, n, ret;
        kstring_t taxonomy = {0, 0, 0};
        kstring_t assign   = {0, 0, 0};

        double value;
        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            
            fields = ksplit(&kt, '\t', &n);
            value = atof(kt.s + fields[ col ]);
            
            taxonomy.l = 0;
            kputs(kt.s + fields[n-1], &taxonomy);

            assign.l = 0;
            if(taxonomy.s[1] != ':')
                kputs(kt.s, &assign);
            else
                abund_assign_value(&taxonomy, &assign, level);

            k = kh_get(double, h, assign.s);
            if(k == kh_end(h)){
                t = kh_put(double, h, assign.s, &ret);
                kh_key(h, t)  = strdup(assign.s);
                kh_val(h, t)  = value;
            }else
                kh_val(h, k) += value;
        }

        abund_print_abundance( h );
        free(taxonomy.s);
        free(assign.s);
        free(kt.s);

        ks_destroy(ks);
        gzclose(fp);
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind]);
        exit(1);
    }

    kh_double_destroy( h );
    kh_char_destroy( map );

    return 0;
}

void abund_assign_value (kstring_t *kt, kstring_t *assign, char level){

    int *fields, i, n;
    fields = ksplit(kt, ',', &n);

    for (i = 0; i < n; ++i){
        if( (kt->s + fields[i])[0] == level ){
            kputs(kt->s + fields[i] + 2, assign);
            break; 
        }
    }

    if(assign->l == 0 ){
       
       khint_t k;
       k = kh_get(char, map, level);
       if(k == kh_end(map)){
             fprintf(stderr, "[ERR]: no such level: %c\n" , level );
             exit(-1);
       }else{
           ksprintf(assign, "Above_%s", kh_val(map, k));
       }
    }

}

void abund_print_abundance (khash_t(double) *h){
    
    khint_t k;
    if  (h == 0) return;
    printf("#taxonomy\tabundance\n");

    for (k = 0; k < kh_end(h); ++k)
        if (kh_exist(h, k)){
            if(kh_val(h, k) == 0) continue;
            printf("%s\t%.4g\n", kh_key(h, k), kh_val(h, k));
        }
}

void kh_abund_init(){
 
    kstring_t kt = {0, 0, 0};
    ksprintf(&kt, "s species g genus f family o order c class p phylum d superkingdom k superkingdom");

    int *fields, i, n, ret;
    khint_t  k;
    fields = ksplit(&kt, ' ', &n);

    for (i = 0; i < n - 1; i += 2){
        k = kh_put(char, map, (kt.s + fields[i])[0] , &ret);
        kh_key(map, k) = (kt.s + fields[i])[0];
        kh_val(map, k) = strdup(kt.s + fields[i + 1]);
    }

}
