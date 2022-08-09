#include "utils.h"

static khash_t(reg32)  *samples_h;
static khash_t(set)    *uniques_h;

#include "kvec.h"
static kvec_t(const char*) samples_t;
static kvec_t(const char*) uniques_t;

KHASH_MAP_INIT_STR(uniques_map,  int *)
static khash_t(uniques_map)   *map_h;
static int n_sample = 0;

void kh_uniques_map_destroy(khash_t(uniques_map) *h){

    khint_t k;
    if  (h == 0) return;
    for (k = 0; k < kh_end(h); ++k) {
        if (kh_exist(h, k)) {
                       
            free((char*)kh_key(h, k));
            free((int*)kh_val(h, k));

        }
    }
    kh_destroy(uniques_map, h);

}

void unique_samples (khash_t(reg32) *samples_h, const char *optarg){

    khint_t k;
    gzFile fp;
    fp = strcmp(optarg, "-")? gzopen(optarg, "r") : gzdopen(fileno(stdin), "r"); 
    if( fp ){
        
        kstream_t *ks;
        ks = ks_init(fp);
        kstring_t kt = {0, 0, 0};
        
        int idx = 0;
        int ret;
        
        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){

            if(kt.s[0] == '#') continue;
            ks_tokaux_t aux;
            kstrtok(kt.s, "\t", &aux);
            kt.s[aux.p - kt.s] = '\0';

            n_sample++;
            k = kh_put(reg32, samples_h, kt.s, &ret);
            if(ret){
               char *p =  strdup(kt.s);
               kh_key(samples_h, k) = p;
               kh_val(samples_h, k) = idx++;
               kv_push(const char *, samples_t, p);
            }

        }
        free(kt.s);
        ks_destroy(ks);
        gzclose(fp);

    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", optarg);
        exit(1);
    }

}

void unique_map (khash_t(uniques_map) *h, khash_t(reg32) *samples_h, khash_t(set) *uniques_h, const char *optarg){

    khint_t k, v;
    gzFile fp;
    fp = strcmp(optarg, "-")? gzopen(optarg, "r") : gzdopen(fileno(stdin), "r"); 
    if( fp ){
        
        kstream_t *ks;
        ks = ks_init(fp);
        kstring_t kt = {0, 0, 0};       
        int *fields, n, ret;

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){

            fields = ksplit(&kt, '\t', &n);

            ks_tokaux_t aux;
            kstrtok(kt.s, ";", &aux);
            kt.s[fields[1] - 2]  = '\0';

            k = kh_put(uniques_map, h, kt.s + fields[1], &ret);

            if(ret){
                char *p        = strdup(kt.s + fields[1]);
                kh_key(h, k)   = p;
                kv_push(const char *, uniques_t, p);
                int *counts    = (int *)calloc(sizeof(int), n_sample);
                kh_val(h, k)   = counts;

            }

            v = kh_get(reg32, samples_h, aux.p +  8);
            if(v != kh_end(samples_h))
                kh_val(h, k)[ kh_val(samples_h, v) ]++;
            else{
               fprintf(stderr, "[ERR]: can't map sample: %s\n", aux.p +  8);
               exit(-1);
            }

        }
        free(kt.s);
        ks_destroy(ks);
        gzclose(fp);

    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", optarg);
        exit(1);
    }

}

void unique_table (khash_t(uniques_map) *h){

   fputs("#OTU table", stdout);
   int i;
   for (i = 0; i < n_sample; ++i)printf("\t%s", kv_A(samples_t, i));
   fputc('\n', stdout);

   khint_t k;
   for (i = 0; i < kv_size(uniques_t); ++i){
      k  = kh_get(uniques_map, h, kv_A(uniques_t, i));
      if(k != kh_end(h)){
         fputs(kh_key(h, k), stdout);
         int j;
         for (j = 0; j < n_sample; ++j)printf("\t%d", kh_val(h, k)[j]);
         fputc('\n', stdout);
      }
   }

}

int unique_table_main(int argc, char *argv[]){

    if ( optind == argc || argc != optind + 2) {
        fprintf(stderr, "\nUsage: atlas-utils uniques_table [options] <mapping_file> <maps>\n\n");
        return 1;
    }

    map_h      = kh_init(uniques_map);
    samples_h  = kh_init(reg32);
    uniques_h  = kh_init(set);

    unique_samples(samples_h, argv[optind]);
    unique_map(map_h, samples_h, uniques_h, argv[optind + 1]);
    unique_table(map_h);

    kh_uniques_map_destroy(map_h);
    kh_set_destroy(uniques_h);
    kv_destroy(uniques_t);
    
    kh_reg32_destroy(samples_h);
    kv_destroy(samples_t);

    return 0;
}