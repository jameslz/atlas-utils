#include "utils.h"

KHASH_MAP_INIT_STR(split, Map)

void kh_split_destroy(khash_t(split) *h);
void split_usage();

typedef struct {
    int   length;
    char *left;
    char *right;
    char *barcode;
    char *directory;
    int   setting;
} opt_t;

void split_opt_init(opt_t *opt);
void split_opt_inspect(opt_t *opt);
void split_make_fs(opt_t *opt);

static khash_t(split) *h;
static khash_t(kreg) *barcode_h;

void split_load_barcode( opt_t *opt,  khash_t(split) *h, khash_t(kreg) *barcode_h );
void split ( opt_t *opt,  khash_t(split) *h, khash_t(kreg) *barcode_h );

int split_main (int argc, char *argv[]){
   
    opt_t opt;
    split_opt_init(&opt);
    int c;

    if(argc == 1) split_usage();

    while ((c = getopt(argc, argv, "1:2:b:l:d:h")) >= 0) {
        
        if (c == '1') {
             opt.left = optarg;
             opt.setting++;
        }else if  (c == '2') {
             opt.right = optarg; 
             opt.setting++;
        }else if (c == 'b') {
             opt.barcode = optarg;
             opt.setting++;
        }else if (c == 'd') {
             opt.directory = optarg;
             opt.setting++;
        }else if (c == 'l') {
             opt.length = atoi(optarg);
        }else if (c == 'h') {
            split_usage();
        }

    }

    split_opt_inspect(&opt);
    split_make_fs(&opt);


    h = kh_init(split);
    barcode_h = kh_init(kreg);

    split_load_barcode(&opt, h, barcode_h);
    split(&opt, h, barcode_h);

    kh_split_destroy(h);
    kh_kreg_destroy(barcode_h);

    return 0;
}

void split_opt_init(opt_t *opt){
    
    memset(opt, 0, sizeof(opt_t));
    opt->length    = 5;
    opt->left      = NULL;
    opt->right     = NULL;
    opt->barcode   = NULL;
    opt->directory = NULL;
    opt->setting   = 0;

}

void split_opt_inspect(opt_t *opt){

     if(opt->left == NULL)      fprintf(stderr, "please specify forward reads file, -1 STR!\n\n"); 
     if(opt->right == NULL)     fprintf(stderr, "please specify reverse reads file, -2 STR!\n\n");
     if(opt->barcode == NULL)   fprintf(stderr, "please barcode mapping file, -b STR!\n\n");
     if(opt->directory == NULL) fprintf(stderr, "please specify output directory, -d STR!\n\n");
    
     if(opt->setting != 4){
        split_usage();
        exit(-1);
     }
}

void split_usage(){

    fprintf(stderr, "\nUsage: atlas-utils split [options]\n\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -1 STR  forward reads file;\n");
    fprintf(stderr, "  -2 STR  reverse reads file;\n");
    fprintf(stderr, "  -b STR  barcode mapping file;\n");
    fprintf(stderr, "  -d STR  output filename directory;\n");
    fprintf(stderr, "  -l INT  minimum barcode length, default: 5;\n");
    fprintf(stderr, "  -h      display usage;\n");
    fprintf(stderr, "\nExample:\natlas-utils split -b barcodes.txt -1 I365.R1.fastq.gz  -2  I365.R2.fastq.gz -l 5  -d  I365\n\n");

    exit(-1);
}

void kh_split_destroy(khash_t(split) *h){
    
    khint_t k;
    if  (h == 0) return;
    for (k = 0; k < kh_end(h); ++k){
        if (kh_exist(h, k)){
            Map map = kh_val(h, k);
            free( (char*)kh_key(h, k) );
            free( map.fs );
            free( map.rs );
            fclose( map.fh );
            fclose( map.rh );
        }
    }
    kh_destroy(split, h);
}

void split_make_fs(opt_t *opt){

    kstring_t kv = {0, 0, 0};
    ksprintf(&kv, "mkdir -p %s", opt->directory);
    if(  system(kv.s) != 0 ) exit (-1);
    free(kv.s);

}

void split_load_barcode( opt_t *opt,  khash_t(split) *h, khash_t(kreg) *barcode_h) {

    gzFile     fp;    
    fp = strcmp(opt->barcode, "-")? gzopen(opt->barcode, "r") : gzdopen(fileno(stdin), "r"); 

    if (fp) {

        kstream_t *ks;
        ks = ks_init(fp);
        kstring_t kt = {0, 0, 0};
        
        kstring_t kk = {0, 0, 0};
        kstring_t kv = {0, 0, 0};

        kstring_t linker = {0, 0, 0};
        
        int *fields, n, ret;
        khint_t k;

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
            
            if(kt.s[0] == '#') continue; 
            fields  = ksplit(&kt, '\t', &n);

            if(n != 3){
                fprintf(stderr, "[ERR]: barcode file format error!\n");
                exit(1);
             }

            if(strlen(kt.s + fields[1]) < opt->length || strlen(kt.s + fields[2]) < opt->length ){
                fprintf(stderr, "[ERR]: barcode seed length should shorter than barcode sequence!\n");
                exit(1);
            }

            kk.l = kv.l = 0;

            ksprintf(&kk, "%s/%s_1.fastq", opt->directory, kt.s);
            ksprintf(&kv, "%s/%s_2.fastq", opt->directory, kt.s);

            FILE *fv = fopen(kk.s, "w");
            FILE *rv = fopen(kv.s, "w");
            
            Map map  = {strdup(kt.s + fields[1]),
                        strdup(kt.s + fields[2]),
                        strlen(kt.s + fields[1]),
                        strlen(kt.s + fields[2]),
                        fv,
                        rv};

            k = kh_put(split, h, kt.s, &ret);
            kh_key(h, k)  = strdup( kt.s );
            kh_val(h, k)  = map;

            linker.l = 0;
            kputsn(kt.s + fields[1], opt->length, &linker);
            kputc('+', &linker);
            kputsn(kt.s + fields[2], opt->length, &linker);

            k = kh_get(kreg, barcode_h, linker.s);
            if(k == kh_end(barcode_h)){
                k = kh_put(kreg, barcode_h, linker.s , &ret);
                kh_key(barcode_h, k)  = strdup( linker.s );
                kstring_t ktmp        = {0, 0, 0};
                kputs(kt.s, &ktmp);
                kh_val(barcode_h, k)  = ktmp;
            }else{             
                kputc('+',  &kh_val(barcode_h, k));
                kputs(kt.s, &kh_val(barcode_h, k));
            }

         }

         if(kk.s != NULL ) free(kk.s);
         if(kt.s != NULL ) free(kt.s);
         if(kv.s != NULL ) free(kv.s);
         if(linker.s != NULL ) free(linker.s);

         ks_destroy(ks);
         gzclose(fp);

    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", opt->barcode);
        exit(1);
    }


}

void split( opt_t *opt,  khash_t(split) *h, khash_t(kreg) *barcode_h ){


    gzFile fp_fv, fp_rv;
    fp_fv = strcmp(opt->left, "-")? gzopen(opt->left, "r") : gzdopen(fileno(stdin), "r"); 
    fp_rv = strcmp(opt->right, "-")? gzopen(opt->right, "r") : gzdopen(fileno(stdin), "r"); 

    if(fp_fv && fp_rv){

        kseq_t *kseq[2];
        kseq[0] = kseq_init(fp_fv);
        kseq[1] = kseq_init(fp_rv);

        kstring_t linker = {0, 0, 0};
        kstring_t kk     = {0, 0, 0};
        kstring_t kv     = {0, 0, 0};

        khint_t   k, t;
        int *fields, i, n;
        
        while (kseq_read(kseq[0]) >= 0) {
            
            if (kseq_read(kseq[1]) < 0) {
                fprintf(stderr, "[W::%s]: the 2nd file has fewer records.\n", __func__);
                break;
            }

            //forwards
            
            linker.l = 0;
            kputsn(kseq[0]->seq.s, opt->length, &linker); kputc('+', &linker); kputsn(kseq[1]->seq.s, opt->length, &linker);
            k = kh_get(kreg, barcode_h, linker.s);
            if( k != kh_end(h) ){

                  fields = ksplit(&kh_val(barcode_h, k), '+', &n);

                  for (i = 0; i < n; ++i){
                       
                       t = kh_get(split, h, kh_val(barcode_h, k).s + fields[i]);
                       if(t == kh_end(h)) {
                           fprintf(stderr, "[ERR]: barcode map error!\n");
                           exit(1);
                       }else{
                            
                            kk.l = kv.l = 0;
                            kputsn(kseq[0]->seq.s, kh_val(h, t).fl , &kk);
                            kputsn(kseq[1]->seq.s, kh_val(h, t).rl,  &kv);

                            if(strcmp(kk.s, kh_val(h, t).fs) == 0 && strcmp(kv.s, kh_val(h, t).rs) == 0 ){
                                print_kseq(kseq[0], kh_val(h, t).fh);
                                print_kseq(kseq[1], kh_val(h, t).rh);
                            }
                            
                       }

                  }
            }

            //reverse

            linker.l = 0;
            kputsn(kseq[1]->seq.s, opt->length, &linker); kputc('+', &linker); kputsn(kseq[0]->seq.s, opt->length, &linker);
            k = kh_get(kreg, barcode_h, linker.s);
            if( k != kh_end(h) ){

                  fields = ksplit(&kh_val(barcode_h, k), '+', &n);

                  for (i = 0; i < n; ++i){
                       t = kh_get(split, h, kh_val(barcode_h, k).s + fields[i]);
                       if(t == kh_end(h)) {
                           fprintf(stderr, "[ERR]: barcode map error!\n");
                           exit(1);
                       }else{
                            
                            kk.l = kv.l = 0;
                            kputsn(kseq[0]->seq.s, kh_val(h, t).rl , &kk);
                            kputsn(kseq[1]->seq.s, kh_val(h, t).fl,  &kv);

                            if(strcmp(kk.s, kh_val(h, t).rs) == 0 && strcmp(kv.s, kh_val(h, t).fs) == 0 ){
                                print_kseq(kseq[0], kh_val(h, t).fh);
                                print_kseq(kseq[1], kh_val(h, t).rh);
                            }
                            
                       }

                  }
            }

        }
        
        if (kseq_read(kseq[1]) >= 0)
            fprintf(stderr, "[W::%s]: the 1st file has fewer records.\n", __func__);
        
         if(kk.s != NULL ) free(kk.s);
         if(kv.s != NULL ) free(kv.s);
         if(linker.s != NULL ) free(linker.s);

        kseq_destroy(kseq[0]); gzclose(fp_fv);
        kseq_destroy(kseq[1]); gzclose(fp_rv);
    }

}
