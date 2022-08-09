#include "utils.h"

typedef struct{
    int  count;
    kstring_t abundance;
} Level;

KHASH_MAP_INIT_STR(level, Level *)
static khash_t(char) *map;

#include "kvec.h"
static kvec_t(double) dv;
static int cnt;
static int abb   =  2;

void level_assign_value(kstring_t *kt, kstring_t *assign, char level);
void level_dv_zero (void);
void level_dv_print (int count, kstring_t *ks);
void level_print_abundance (khash_t(level) *h, kstring_t *title);

void kh_level_destroy(khash_t(level) *h);
void kh_level_init(void);

int level_main(int argc, char *argv[]){
    
    char level = 'g';
    int c;
    while ((c = getopt(argc, argv, "l:n")) >= 0) {
        if (c == 'l') level = optarg[0];
        else if(c == 'n') abb = 0;
    }

    if ( optind == argc || argc != optind + 1) {

        fprintf(stderr, "\nUsage: atlas-utils level [options] <otutab:annotated>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -l  taxonomy level in [dkcofgs] Default:['g']\n");
        fprintf(stderr, "  -n  print the raw label. such as 'p:Firmicutes'\n\n");
        return 1;
    } 

    khash_t(level) *h;
    h = kh_init(level);

    map = kh_init(char);
    kh_level_init();

    kv_init(dv);
    khint_t k, t;

    gzFile fp;
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r");
    if (fp) {

        kstream_t *ks;
        ks = ks_init(fp);

        kstring_t kt    = {0, 0, 0};
        int *fields, n, i, ret;

        kstring_t title = {0, 0, 0};

        if( ks_getuntil( ks, '\n', &kt, 0) ){
            if(kt.s[0] == '#'){                
                kputs("#level", &title);
                fields = ksplit(&kt, '\t', &n);
                cnt = n - 2;
                for (i = 1; i < n - 1; ++i) ksprintf(&title, "\t%s", kt.s + fields[i]);
            }
        }

        kstring_t taxonomy = {0, 0, 0};
        kstring_t assign   = {0, 0, 0};
        kstring_t values   = {0, 0, 0};

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){

            fields = ksplit(&kt, '\t', &n);
            
            taxonomy.l = 0;
            kputs(kt.s + fields[n-1], &taxonomy);

            assign.l = 0;
            if(taxonomy.s[1] != ':')
                kputs(taxonomy.s, &assign);
            else
                level_assign_value(&taxonomy, &assign, level);

            values.l = 0;
            for (i = 1; i < n - 1; ++i){
                if(i != 1) kputc('\t', &values) ;
                kputs(kt.s + fields[i], &values);
            }

            k = kh_get(level, h, assign.s);
            if(k == kh_end(h)){
                t = kh_put(level, h, assign.s, &ret);
                
                kh_key(h, t)  = strdup(assign.s);
                Level *level  = (Level *)malloc( sizeof(Level) );
                kstring_t k_tmp  = {0, 0, 0};
                kputs(values.s , &k_tmp);
                if( level != NULL ){
                    level->count      = 1;
                    level->abundance  = k_tmp;
                }
                kh_val(h, t)  =  level;
            
            }else{
                kh_val(h, k)->count++;
                kputc('\t', &kh_val(h, k)->abundance);
                kputs(values.s, &kh_val(h, k)->abundance);
            }

        }
        
        level_print_abundance(h, &title);

        free(taxonomy.s);
        free(assign.s);
        free(values.s);
        free(kt.s);
        free(title.s);

        ks_destroy(ks);
        gzclose(fp);
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind]);
        exit(1);
    }

    kh_level_destroy( h );
    kv_destroy(dv);
    kh_char_destroy( map );
    return 0;
}

void level_assign_value(kstring_t *kt, kstring_t *assign, char level){

    int *fields, i, n;
    
    fields = ksplit(kt, ',', &n);
    
    for (i = 0; i < n; ++i){
        if( (kt->s + fields[i])[0] == level ){
            kputs(kt->s + fields[i] + abb, assign);
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

void level_dv_zero (void){
    int i;
    for (i = 0; i < cnt; ++i) kv_a(double, dv,  i) = 0.0; 
}

void level_dv_print (int count, kstring_t *kt){

    int *fields, i, j, n;
    
    fields = ksplit(kt, '\t', &n);
    level_dv_zero();

    for (i = 0; i < cnt; ++i)
        for (j = 0; j < count; ++j)
            kv_a(double, dv,  i) += atof(kt->s + fields[i + j * cnt]);
    for (i = 0; i < cnt; ++i) printf("\t%.4g", kv_A(dv,  i));

}

void level_print_abundance (khash_t(level) *h, kstring_t *title){
    
    khint_t k;
    if  (h == 0) return;

    printf("%s\n", title->s);

    for (k = 0; k < kh_end(h); ++k) {
        if (kh_exist(h, k)) {
            printf("%s", kh_key(h, k) );
            level_dv_print(kh_val(h, k)->count,  &kh_val(h, k)->abundance);
            printf("\n");
        }
    }
}

void kh_level_destroy(khash_t(level) *h){
    
    khint_t k;
    if  (h == 0) return;
    for (k = 0; k < kh_end(h); ++k) {
        if (kh_exist(h, k)) {
            free( kh_val(h, k)->abundance.s );
            free( kh_val(h, k) );
        }
    }
    kh_destroy(level, h);
}

void kh_level_init() {
    
    kstring_t kt = {0, 0, 0};
    ksprintf(&kt, "s species g genus f family o order c class p phylum d superkingdom k superkingdom");

    int *fields, i, n, ret;
    khint_t  k;
    fields = ksplit(&kt, ' ', &n);

    for (i = 0; i < n - 1; i += 2){
        k = kh_put(char, map, (kt.s + fields[i])[0], &ret);
        kh_key(map, k) = (kt.s + fields[i])[0];
        kh_val(map, k) = strdup(kt.s + fields[i + 1]);
    }

}

