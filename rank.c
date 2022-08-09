#include "utils.h"

KHASH_MAP_INIT_INT(32, char*)
void kh_rank_destroy(khash_t(32) *h);

khash_t(kreg) *repo;

void insert(khash_t(kreg) *h, kstring_t *kt, int line);

#include "ksort.h"
#define khint_lt(a, b) (atof( kh_key(repo, (a)) ) > atof( kh_key(repo, (b))))
KSORT_INIT(khint, khint_t, khint_lt)

inline void titles (kstring_t *title);

static int n_sample;
void print_others(double others[n_sample]);
void add_others(double others[n_sample], char *str);


int rank_main(int argc, char *argv[]){

    int c;
    int m = 0;
    int R = 20;
    int I = 0;

    while ((c = getopt(argc, argv, "ar:m")) >= 0) {
        
        if (c == 'r')  R = atoi(optarg);
        else if (c == 'm'){
            m = 1;
        }
        else if (c == 'a'){
            I = 1;
        }
    
    }

    if(m) R--;

    if ( optind == argc || argc != optind + 1 ) {

        fprintf(stderr, "\n");
        fprintf(stderr, "Usage: atlas-utils rank [options] <tsv>\n\n");
        fprintf(stderr, "Options: -r  INT top catalogs.\n");
        fprintf(stderr, "         -m      merge remain to others level.\n");
        fprintf(stderr, "         -a      merge above_ to others level.\n\n");        
        return 1;
    
    }

    kstring_t kt    = {0, 0, 0};
    kstring_t title = {0, 0, 0};

    int *fields, i, ret, n, line;
    line = 0;

    khash_t(32) *h;
    h = kh_init(32);
    khint_t k;

    repo = kh_init(kreg);

    kstream_t *ks;

    gzFile fp;
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r"); 

    if(fp){
        
        ks  = ks_init(fp);
        if(ks_getuntil( ks, '\n', &kt, 0) >= 0 ){
            if(kt.s[0] == '#'){
                kputs(kt.s, &title);
                ksplit(&kt, '\t', &n_sample);
                n_sample--;
            }else{
                line++;
                k = kh_put(32, h, line, &ret);
                if(ret == 1) kh_val(h, k) = strdup(kt.s); 
                insert(repo, &kt, line);
            }
        }else{
            fprintf(stderr, "[ERR]: empty file %s\n", argv[optind]);
            exit(1);
        }

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){

            line++;
            k = kh_put(32, h, line, &ret);
            if(ret == 1) kh_val(h, k) = strdup(kt.s); 
            insert(repo, &kt, line);
        
        }
        ks_destroy(ks);
        gzclose(fp);
    
    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind]);
        exit(1);
    }

    titles( &title );

    int N = kh_size(repo);
    khint_t *khint = (khint_t *) malloc(N * sizeof(khint_t));

    i = 0;
    for (k = 0; k < kh_end(repo); ++k) if (kh_exist(repo, k)) khint[i++] = k;
    ks_mergesort(khint, N, khint, 0);
   
    khash_t(32) *print_h;
    print_h = kh_init(32);

    double others[n_sample];
    memset(others, 0, sizeof(others[0]) * n_sample);
    kstring_t aux = {0, 0, 0};

    for (i = 0; i < N; ++i){
        
        if(R <= 0 && m != 1) break;
        k = khint[i];
        aux.l = 0;
        kputs( kh_val(repo, k).s , &aux);
        fields = ksplit(&aux, ';', &n);

        int j;
        for (j = 0; j < n; ++j){

            int hit = atoi(aux.s + fields[j]);
            khint_t t = kh_get(32, h, hit);
            if( t != kh_end(h)){
                
                kh_put(32, print_h, hit, &ret);
                if(ret == 0) continue;

                if(R > 0){
                    if( strncmp(kh_val(h, t), "Above_", 6) == 0 && I == 1 ){
                        add_others(others, kh_val(h, t)); 
                    }else{
                        printf("%s\n",  kh_val(h, t));
                        R--;
                    }
                }else{
                    if(m) add_others(others, kh_val(h, t));    
                }
            }
        }
    }

    if(m && R <= 0) print_others(others);

    kh_destroy(32, print_h);
    kh_rank_destroy( h );
    kh_kreg_destroy( repo );
    free(title.s);
    return 0;
}

void add_others (double others[n_sample], char *str){

    int k;
    ks_tokaux_t aux;
    char *p;
    int i = 0;   
    for (p = kstrtok(str, "\t", &aux), k = 0; p; p = kstrtok(0, 0, &aux), ++k){
         if(p == str) continue;
         str[aux.p - str] = '\0';
         others[i++] += atof(p);
    }

}

void print_others (double others[n_sample]){

    fputs("Others", stdout);
    int i;
    for (i = 0; i < n_sample; ++i){
        fputc('\t', stdout);
        fprintf(stdout, "%.4g", others[i]);
    }
    fputc('\n', stdout);

}

void kh_rank_destroy(khash_t(32) *h){
    
    khint_t k;
    if  (h == 0) return;
    for (k = 0; k < kh_end(h); ++k) {
        if (kh_exist(h, k))  free((char*)kh_val(h, k));
    }
    kh_destroy(32, h);

}

inline void titles (kstring_t *kt){

    if(kt-> l != 0) printf("%s\n", kt->s);

}

void insert(khash_t(kreg) *h, kstring_t *kt, int line){

    int *fields, i, n, ret;
    khint_t k;
    fields = ksplit(kt, '\t', &n);
    for (i = 1; i < n; ++i){
        k = kh_put(kreg, h, kt->s + fields[i], &ret);
        if(ret == 1){

            kstring_t k_tmp = {0, 0 ,0};
            ksprintf(&k_tmp, "%d",  line);
            kh_val(h, k) = k_tmp;
            kh_key(h, k) = strdup(kt->s + fields[i]);

        }else if(ret == 0) ksprintf(&kh_val(h, k), ";%d",  line);

    }

}