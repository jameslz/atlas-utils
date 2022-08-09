#include "utils.h"

void krona_print (khash_t(double) *h, khash_t(reg) *map);
int  krona_main(int argc, char *argv[]){
    
    if ( argc != 3 ) {
        
        fprintf(stderr, "\nUsage: atlas-utils krona <otutab:annotated> <sample>\n\n");
        return 1;
    
    }

    khash_t(double) *h;
    h = kh_init(double);

    khash_t(reg) *map;
    map = kh_init(reg);
    
    gzFile fp;
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r");
    if (fp) {

        kstream_t *ks;
        ks = ks_init(fp);

        kstring_t kt    = {0, 0, 0};
        ks = ks_init(fp);
        int col      = 0;

        if( ks_getuntil( ks, '\n', &kt, 0)  ){

            if(kt.s[0] == '#'){
                col = get_target_column(&kt, argv[optind + 1]);
            }else{
                fprintf(stderr, "[ERR]: no title line start with '#' ? \n");
                return 0;
            }

        }else{
            fprintf(stderr, "[ERR]: file empty? \n");
            return 0;
        }
        
        int i, *fields, n, *level, m, ret;
        kstring_t taxonomy = {0, 0, 0};
        kstring_t lineage  = {0, 0, 0};
        
        khint_t k;
        double value;
        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            
            fields = ksplit(&kt, '\t', &n);
            value = atof(kt.s + fields[ col ]);
            
            taxonomy.l = 0;
            kputs(kt.s + fields[n-1], &taxonomy);

            for (i = 0; i < taxonomy.l; ++i) if(taxonomy.s[i] == '"') taxonomy.s[i] = ' ';

            lineage.l  = 0;
            level = ksplit(&taxonomy, ',', &m);
            for (i = 0; i < m; ++i){
                
                ksprintf(&lineage, "\t%s", taxonomy.s + level[i]);
                
                if(i == m - 1) continue;
                k = kh_put(reg, map, lineage.s, &ret);
                if( ret ){
                    kh_key(map, k)  = strdup(lineage.s);
                    kh_val(map, k)  = strdup(taxonomy.s + level[i]);
                }
            }

            k = kh_put(double, h, lineage.s, &ret);
            if(ret){
                kh_key(h, k)  = strdup(lineage.s);
                kh_val(h, k)  = value;
            }else if(ret == 0){
                kh_val(h, k) += value;
            }

        }
        
        free(taxonomy.s);
        free(lineage.s);
        free(kt.s);

        ks_destroy(ks);
        gzclose(fp);

    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind]);
        exit(1);

    }

    krona_print(h, map);
    kh_double_destroy( h );
    kh_reg_destroy( map );

    return 0;
}

void krona_print (khash_t(double) *h, khash_t(reg) *map){
    
    khint_t k, t;
    if  (h == 0) return;
    for (k = 0; k < kh_end(h); ++k) {
       if (kh_exist(h, k)) {
           if( kh_val(h, k) == 0) continue;
           t = kh_get(reg, map, kh_key(h, k));
           (t == kh_end(map)) ? printf("%g%s\n", kh_val(h, k), kh_key(h, k)) :
                                printf("%g%s\t%s_unclassified\n", kh_val(h, k), kh_key(h, k), kh_val(map, t) + 2);
       }
    }
}