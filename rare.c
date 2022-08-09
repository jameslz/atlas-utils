#include "utils.h"
#include "fastrand.h"
#include <limits.h>
#include <unistd.h>

#include "kvec.h"
static kvec_t( char * ) vt;

void rarefaction (const uint32_t *, uint32_t *, const int, const int, const uint32_t);

int rare_main(int argc, char *argv[]){

    int c;
    uint32_t i, j, seed = 11, num = 0;

    while ((c = getopt(argc, argv, "s:")) >= 0)
        if (c == 's') seed = atol(optarg);

    if (optind + 2 > argc) {
        fprintf(stderr, "\n");
        fprintf(stderr, "Usage: atlas-utils rare [-s seed=11] <otutab> <sample_size>\n\n");
        fprintf(stderr, "Options: -s INT    RNG seed [11]\n\n");
        return 1;
    }
   
    pcg32_init_state(seed);
     
    num = atol( argv[optind + 1] );

    int otu_num = 0, size = 0;
    kstring_t  kt    = {0, 0, 0};
    kstream_t  *ks;
    kstring_t title  = {0, 0, 0};

    gzFile  fp; 
    fp = strcmp(argv[ optind ], "-")? gzopen(argv[ optind ], "r") : gzdopen(fileno(stdin), "r"); 
    if( fp ){

        ks = ks_init(fp);
        if( ks_getuntil( ks, '\n', &kt, 0) >=  0 ){
            
            if( kt.s[0] == '#'){
                kputs(kt.s, &title);
                ksplit(&kt, '\t', &size);
                size = size - 1;
            }else{
                fprintf(stderr, "[ERR]: no title fields, exit.\n");
                exit(-1);
            }
        }
        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            if( kt.l == 0 ) continue;
            otu_num++;
        }
        ks_destroy(ks);
        gzclose(fp);
    
    }else{        
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[ optind ]);
        exit(1);
    }
    
    if(!otu_num){
        fprintf(stderr, "[ERR]: empty otutab.?\n");
        exit(-1);
    }

    uint32_t *abund[size], *norm[size], tot[size];
    for (i = 0; i < size; ++i){
        abund[i]   = (uint32_t *) calloc(otu_num, sizeof(uint32_t));
        norm[i]    = (uint32_t *) calloc(otu_num, sizeof(uint32_t));
        tot[i]     = 0;
    }

    int *fields, n;
    fp = strcmp(argv[ optind ], "-")? gzopen(argv[ optind ], "r") : gzdopen(fileno(stdin), "r"); 
    if( fp ){
        
        int line = 0;
        ks = ks_init(fp);
        kv_init(vt);
 
        if( ks_getuntil( ks, '\n', &kt, 0) >=  0 ) printf("%s\n", kt.s);

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            if( kt.l == 0 || kt.s[0] == '#' ) continue;
            fields =  ksplit(&kt, '\t', &n);
            kv_push(char*, vt, strdup(kt.s));          
            for (i = 1; i < n; ++i){
                abund[i - 1][line] = atoi(kt.s + fields[i]);
                tot[i - 1] += abund[i - 1][line];
            }
            ++line;
        }
        
        ks_destroy(ks);
        gzclose(fp);
    
    }

    fields = ksplit(&title, '\t', &n);
    for (i = 0; i < size; ++i){
        if(tot[i] < num){
            fprintf(stderr, "[ERR]: The sample %s with total reads: %d less the rare sample size: %d\n", title.s + fields[i + 1] , tot[i], num);
            exit(-1);
        }
    }
    free(title.s);

    for (i = 0; i < size; ++i) rarefaction(abund[i], norm[i], tot[i], otu_num, num);

    for (i = 0; i < otu_num; ++i){
        printf("%s", kv_A(vt, i) );
        free( kv_A(vt, i) );
        for (j = 0; j < size; ++j) printf("\t%d", norm[j][i]);
        putchar('\n');
    }

    for (i = 0; i < size; ++i){
        if(norm[i]   != NULL)   free(norm[i]);
        if(abund[i]  != NULL)   free(abund[i]);
    }
    
    kv_destroy(vt);
    return 0;

}

void rarefaction (const uint32_t *cnt, uint32_t *norm, const int tot_num, const int otu_num, const uint32_t num){

    uint32_t val[tot_num];
    uint32_t i, j, k = 0;

    for (i = 0; i < otu_num; ++i){
        for (j = 0; j < cnt[i]; ++j){
            val[ k ] = i;
            ++k;
        }
    }

    uint32_t *buf = (uint32_t *)malloc(num * sizeof(uint32_t));
    for (i = 0; i < num; ++i) buf[i] = UINT32_MAX;
    
    uint32_t n_seqs = 0;
    for (i = 0; i < k;  ++i){

        uint32_t r = pcg32_random_bounded_divisionless(n_seqs);
        uint32_t y;
        ++n_seqs;
        y = n_seqs - 1 < num? n_seqs - 1 : r;
        if (y < num) buf[y] = n_seqs;

    }

    for (i = 0; i < otu_num; ++i) norm[i] = 0;
    for (i = 0; i < num; ++i) ++norm[ val[buf[i] - 1] ];

}