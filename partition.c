#include "utils.h"
#include "kvec.h"

static kvec_t( FILE * ) vt;
static int num      = 30;
static int parts    = 1;
static char *prefix = NULL;

void parts_init(kstring_t *kt);
void parts_print(kstring_t *kt);
void parts_destroy();

int partition_main (int argc, char *argv[]){

    int c;
    while ((c = getopt(argc, argv, "n:")) >= 0) {
        if (c == 'n') num = atoi(optarg);
    }

    if ( optind == argc || argc != optind + 2) {
        fprintf(stderr, "\nUsage: atlas-utils partition [options] <otutab> <prefix>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -n  samples per partition: default: [30];\n\n");
        return 1;
    }
    
    kv_init(vt);

    prefix = argv[optind + 1];

    gzFile fp;
    fp = strcmp(argv[ optind ], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r");
    if (fp) {

        kstream_t *ks;
        ks = ks_init(fp);
        kstring_t kt = {0, 0, 0};

        if( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            parts_init(&kt);
        }

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            if(kt.l == 0) continue;
            parts_print(&kt);
        }

        free(kt.s);

    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[1]);
        exit(1);
    }


    parts_destroy();
    kv_destroy(vt);
    
    printf("%d\n", parts);
    return 0;

}

void parts_print(kstring_t *kt){

    int *fields, n, i;
    fields = ksplit(kt,'\t', &n);
    for (i = 0; i < parts; ++i)  fprintf(kv_A(vt, i), "%s", kt->s);
    for (i = 1; i < n; ++i)  fprintf(kv_A(vt, (i - 1)/num), "\t%s", kt->s + fields[i]);
    for (i = 0; i < parts; ++i)  fprintf(kv_A(vt, i), "\n");

}

void parts_init(kstring_t *kt){

    int *fields, n, i;
    fields = ksplit(kt,'\t', &n);
    
    parts  = ( (n - 1) % num == 0) ? 
               (n - 1)/num :
               (n - 1)/num + 1;
    
    kstring_t kp = {0, 0, 0};
    for (i = 0; i < parts; ++i){
        kp.l = 0;
        
        ksprintf(&kp, "%s.%d.txt", prefix, i + 1);
        FILE *fh;

        if (( fh = fopen(kp.s, "w")) == NULL) {
            fprintf(stderr,"[ERR]: can't open file %s\n", kp.s);
            exit(1);
        }
        kv_push(FILE *, vt, fh);

        fprintf(fh, "#OTU ID");

    }

    for (i = 1; i < n; ++i)  fprintf(kv_A(vt, (i - 1)/num), "\t%s", kt->s + fields[i]);
    for (i = 0; i < parts; ++i)  fprintf(kv_A(vt, i), "\n");

    free(kp.s);
}

void parts_destroy(){
    
    int i;
    for (i = 0; i < parts; ++i)  fclose(kv_A(vt, i));

}