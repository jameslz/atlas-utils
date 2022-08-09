#include "utils.h"

#include "kvec.h"
static kvec_t( char * ) vt;

inline void to_lower_case(char *p);
void cut_fileds(char *s);

int filter_main(int argc, char *argv[]){
    
    int  c;
    char *T = NULL;
    int   L = 0;

    while ((c = getopt(argc, argv, "t:r")) >= 0) {
        if (c == 't')  T = optarg;
        else if (c == 'r') L = 1;
    }    

    if ( optind == argc || argc != optind + 1) {

        fprintf(stderr, "\nUsage: atlas-utils filter [options] <sintax>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "   -t STR filter d:Bacteria or d:Archaea, or Others Default:[NONE]\n");
        fprintf(stderr, "          for multi-targer: d:Archaea,p:Firmicutes\n");
        fprintf(stderr, "   -r     print OTU IDs after filter.\n\n");
        return 1;
    }    

    if(T != NULL){
        to_lower_case(T);
        kv_init(vt);
		cut_fileds(T);
    }

    gzFile fp;
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r");
    if (fp) {

        kstream_t *ks;
        ks = ks_init(fp);

        kstring_t kt    = {0, 0, 0};
        kstring_t kv    = {0, 0, 0};

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            
                kv.l = 0;
                kputs(kt.s, &kv);
                to_lower_case(kv.s);

                char *mito, *chlor;

                if( (mito  = strstr(kv.s,  "mitochondria")) != 0 || 
                    (chlor = strstr(kv.s,  "chloroplast"))  != 0 ) continue;

                if( T != NULL ){
                	
                	int f = 0;
                	int i;
                    for (i = 0; i < kv_size(vt); ++i){
                		if( strstr(kv.s,  kv_A(vt, i)) != 0 ){
                			f = 1;
                			break;
                		}
                    }
                    if(f) continue;
                }

                puts(kt.s);

                if(L){
                   ks_tokaux_t aux;
                   kstrtok(kt.s, "\t", &aux);
                   kt.s[aux.p - kt.s] = '\0';
                   fprintf(stderr, "%s\n", kt.s);
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

    if( T != NULL ){
    	int i;
    	for (i = 0; i < kv_size(vt); ++i) free(kv_A(vt, i));
    }
    kv_destroy(vt);
    return 0;

}


inline void to_lower_case(char *p){

    for ( ; *p; ++p) *p = tolower(*p);
}

void cut_fileds(char *s){
    
    kstring_t kt = {0,0,0};
    kputs(s, &kt);
    int *fields, n, i;
    fields = ksplit(&kt, ',', &n);
    for (i = 0; i < n; ++i) kv_push(char *, vt, strdup(kt.s + fields[i]));
    

}