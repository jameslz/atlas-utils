#include "utils.h"

typedef struct {
    int annotation;
    int width;
    char *lablel;
} opt_t;

void uniques_opt_init(opt_t *opt);

#include "khash.h"
KHASH_MAP_INIT_STR(uniques, int)

static khash_t(uniques) *h;

#include "ksort.h"
#define khint_lt(a, b) (kh_val(h, (a)) > kh_val(h, (b)))
KSORT_INIT(uniques, khint_t, khint_lt)

void kh_uniques_destroy(khash_t(uniques) *h);
void uniques_printstr(const char *s, unsigned line_len);

int uniques_main(int argc, char *argv[]){
    
    opt_t opt;
    uniques_opt_init(&opt);
    int c;
    while ((c = getopt(argc, argv, "w:l:a")) >= 0) {
        if (c == 'a') opt.annotation = 1;
        if (c == 'l') opt.lablel=optarg;
        if (c == 'w') opt.width=atoi(optarg);

    }

    if ( optind == argc || argc > optind + 1) {
        fprintf(stderr, "\nUsage: div_utils uniques [options] <in.fa/q>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -a      size annotation\n");
        fprintf(stderr, "  -l STR  sequence label, default: [Uniq]\n");
        fprintf(stderr, "  -w INT  number of residues per line; 0 for 2^32-1 default:[0]\n\n");
        return 1;
    }

    h = kh_init(uniques);
    khint_t k;

    gzFile fp;
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r"); 
    if( fp ){
        kseq_t   *kseq;
        kseq  = kseq_init(fp);
        int   l, ret;
        while ((l = kseq_read(kseq)) >= 0) {
        
            k = kh_put(uniques, h, kseq->seq.s, &ret);
            if(ret == 0)
                kh_val(h, k)++;
            else{
                kh_key(h, k) = strdup(kseq->seq.s);
                kh_val(h, k) = 1;
            }
        
        }

        kseq_destroy(kseq);
        gzclose(fp);

    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind]);
        exit(1);
    }

    int N = kh_size(h);
    khint_t *khint = (khint_t *)malloc(N * sizeof(khint_t));

    int i = 0;
    for (k = 0; k < kh_end(h); ++k)
        if (kh_exist(h, k)) khint[i++] = k;

    ks_mergesort(uniques, N, khint, 0);
    
    for (i = 0; i < N; ++i){
        k = khint[i];
        printf(">%s%d", opt.lablel, i + 1);
        if(opt.annotation) printf(";size=%d;", kh_val(h, k));
        uniques_printstr(kh_key(h, k), opt.width);
    }

    kh_uniques_destroy(h);
    free(khint);
    return 0;

}

void uniques_opt_init(opt_t *opt){
    memset(opt, 0, sizeof(opt_t));
    opt->annotation = 0;
    opt->lablel     = "Uniq";
    opt->width      = 0;
}

void kh_uniques_destroy(khash_t(uniques) *h){
    
    khint_t k;
    if  (h == 0) return;
    for (k = 0; k < kh_end(h); ++k)
        if (kh_exist(h, k))  free((char*)kh_key(h, k));
    
    kh_destroy(uniques, h);
}

void uniques_printstr(const char *s, unsigned line_len){
    
    if (line_len != 0) {
        int l = strlen(s);
        int i, rest = strlen(s);
        for (i = 0; i < l; i += line_len, rest -= line_len) {
            putchar('\n');
            if (rest > line_len) fwrite(s + i, 1, line_len, stdout);
            else fwrite(s + i, 1, rest, stdout);
        }
        putchar('\n');
    } else {
        putchar('\n');
        puts(s);
    }
}