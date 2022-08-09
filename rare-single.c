#include "utils.h"
#include "fastrand.h"
#include <limits.h>
#include <unistd.h>
#include "kvec.h"

int rare_main(int argc, char *argv[]){

    int c;
    uint32_t i, seed = 11 ,num = 0, n_seqs = 0;

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
    n_seqs = 0;
    uint32_t *buf = (uint32_t *)malloc(num * sizeof(uint32_t));
    for (i = 0; i < num; ++i) buf[i] = UINT32_MAX;

    kvec_t( const char* ) vs;
    kv_init(vs);

    kvec_t( uint32_t ) val;
    kv_init(val);

    int n, *fields;
    uint32_t k, tot = 0, size = 0;
    kstring_t  title  = {0, 0, 0};

    gzFile  fp; 
    fp = strcmp(argv[ optind ], "-")? gzopen(argv[ optind ], "r") : gzdopen(fileno(stdin), "r"); 
    if( fp ){

        kstream_t  *ks;
        kstring_t  kt     = {0, 0, 0};
        k = 0;

        ks = ks_init(fp);

        if( ks_getuntil( ks, '\n', &kt, 0) >=  0 ){
            
            if( kt.s[0] == '#') kputs(kt.s, &title);
            else{
                fprintf(stderr, "[ERR]: no title fields, exit.\n");
                exit(-1);
            }

        }
        
        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){

            if( kt.l == 0 ) continue;
            fields = ksplit(&kt, '\t', &n);
            
            kv_push( const char *, vs, strdup(kt.s));

            uint32_t abund = atol(kt.s + fields[1]);
            tot += abund;
            ++size;

            for (i = 0; i < abund; ++i) kv_push( uint32_t, val, k);
            ++k;

        }

        free(kt.s);
        ks_destroy(ks);
        gzclose(fp);
    
    }else{
        
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[ optind ]);
        exit(1);
    
    }

    if(tot < num){
        fprintf(stderr, "[ERR]: the total reads: %d less the rare sample size: %d\n", tot, num);
        exit(-1);
    }

    if(kv_size(val) == 0) return 0;

    for (i = 0; i < tot;  ++i){

        uint32_t r = pcg32_random_bounded_divisionless(n_seqs);
        uint32_t y;
        ++n_seqs;
        y = n_seqs - 1 < num? n_seqs - 1 : r;
        if (y < num) buf[y] = n_seqs;

    }
    
    uint32_t *rare = (uint32_t *)malloc(size * sizeof(uint32_t));
    for (i = 0; i < size; ++i) rare[i] = 0;
   
    for (i = 0; i < num; ++i) ++rare[kv_A(val, buf[i])];
    
    puts(title.s);
    for (i = 0; i < size; ++i){
        if(rare[i] == 0) continue;
        printf("%s\t%d\n", kv_A(vs, i), rare[i]);
    }


    for (i = 0; i < kv_size(vs); ++i) free((char*)kv_A(vs, i));
    kv_destroy(vs);
    kv_destroy(val);

    if (buf  != NULL) free(buf);
    if (rare != NULL) free(rare);
    free(title.s);
    return 0;

}