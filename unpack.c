#include "utils.h"

static unsigned int level_tab[] = {
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0,  0,  0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0,  0,  0,  
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0,  0,  0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0,  0,  0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0,  0,  0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0,  0,  0,
      0,   0,   0,   2,   0,   0,   4,   5,   0,   0,  0,  0,  0,  0,  0,  3,
      1,   0,   0,   6,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0,  0,  0,
};

static int n_level = 6;
static kstring_t level = {0, 0, 0};

void unpack_level(char * s, kstring_t *kt){
    
    level.l = kt->l = 0;
    kputs(s, &level);
    int *fields, i, j, n;

    fields = ksplit(&level, ',', &n);
    
    int current, pad;
    current =  pad = 0;

    for (i = 0; i < n; ++i){
       current = level_tab[ (unsigned int)(level.s + fields[i])[0] ];
       if(current > pad){
          for (j = 0; j < current - pad; ++j){
              kputc('\t', kt);
              kputs("NA", kt);
          }
       }
       kputc('\t', kt);
       kputs(level.s + fields[i] + 2, kt);
       pad = current + 1;

    }

    if(n_level > current){
      for (j = 0; j < n_level - current; ++j){
          kputc('\t', kt);
          kputs("NA", kt);
      }        
    }

}

int unpack_main (int argc, char *argv[]){
    
    if ( optind == argc || argc != optind + 1) {
        
        fprintf(stderr, "\nUsage: atlas-utils unpack <sintax>\n\n");
        return 1;
    
    }

    kstring_t kt = {0, 0, 0};
    int *fields, i, n;

    gzFile     fp;    
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r"); 

    if (fp) {

        kstream_t *ks;
        ks  =  ks_init(fp);

        printf("#OTU ID\tkindom\tphylum\tclass\torder\tfamily\tgenus\tspecies\n"); 

        kstring_t flat = {0, 0, 0};
        
        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            
            flat.l = 0;
            fields = ksplit(&kt, '\t', &n);
            printf("%s", kt.s);

            if( n == 3){
                for (i = 0; i < 7; ++i){ printf("\tNA"); }
            }else{
                unpack_level(kt.s + fields[3], &flat);
                printf("%s", flat.s);
            }

            printf("\n");
        }

        free(flat.s);
        ks_destroy(ks);
        gzclose(fp);

    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind]);
        exit(1);
    }

    free(kt.s);
    free(level.s);
    return 0;
}
