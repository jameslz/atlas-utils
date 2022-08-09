#include "utils.h"

KHASH_MAP_INIT_STR(demultiplex, Map *)

void kh_demultiplex_destroy(khash_t(demultiplex) *h);

inline void demultiplex_print_seq(FILE *f, const kseq_t *s, int offset);
void demultiplex_usage();

typedef struct {
    int   length;
    int   setting;
    int   split;
    char *left;
    char *right;
    char *barcode;
    char *directory;
} opt_t;

typedef struct {
    int       size;
    char**    elem;
    kstring_t mem;
} mem_t;

KHASH_MAP_INIT_STR(mem, mem_t *)
static khash_t(mem) *barcode_h;
void kh_mem_destroy(khash_t(mem) *h);


void demultiplex_opt_init(opt_t *opt);
void demultiplex_opt_inspect(opt_t *opt);
void demultiplex_make_fs(opt_t *opt);

static khash_t(demultiplex) *h;

void demultiplex_load_barcode( opt_t *opt,  khash_t(demultiplex) *h, khash_t(mem) *barcode_h );
void demultiplex ( opt_t *opt,  khash_t(demultiplex) *h, khash_t(mem) *barcode_h );

int demultiplex_main (int argc, char *argv[]){
   
    opt_t opt;
    demultiplex_opt_init(&opt);
    int c;

    if(argc == 1) demultiplex_usage();

    while ((c = getopt(argc, argv, "s1:2:b:l:d:h")) >= 0) {
        
        if (c == '1') {
             opt.left = strdup(optarg);
             opt.setting++;
        }else if  (c == '2') {
             opt.right = strdup(optarg); 
             opt.setting++;
        }else if (c == 'b') {
             opt.barcode = strdup(optarg);
             opt.setting++;
        }else if (c == 'd') {
             opt.directory = strdup(optarg);
             opt.setting++;
        }else if (c == 'l') {
             opt.length = atoi(optarg);
        }else if (c == 's') {
             opt.split = 1;
        }else if (c == 'h') {
            demultiplex_usage();
        }

    }

    demultiplex_opt_inspect(&opt);

    demultiplex_make_fs(&opt);

    h = kh_init(demultiplex);
    barcode_h = kh_init(mem);

    demultiplex_load_barcode(&opt, h, barcode_h);
    demultiplex(&opt, h, barcode_h);

    kh_demultiplex_destroy(h);
    kh_mem_destroy(barcode_h);

    return 0;
}

void demultiplex_opt_init(opt_t *opt){

    memset(opt, 0, sizeof(opt_t));
    opt->length    = 5;
    opt->left      = NULL;
    opt->right     = NULL;
    opt->barcode   = NULL;
    opt->directory = NULL;
    opt->setting   = 0;
    opt->split     = 0;

}

void demultiplex_opt_inspect(opt_t *opt){

     if(opt->left == NULL)      fprintf(stderr, "please specify forward reads file, -1 STR!\n\n"); 
     if(opt->right == NULL)     fprintf(stderr, "please specify reverse reads file, -2 STR!\n\n");
     if(opt->barcode == NULL)   fprintf(stderr, "please barcode mapping file, -b STR!\n\n");
     if(opt->directory == NULL) fprintf(stderr, "please specify output directory, -d STR!\n\n");
    
     if(opt->setting != 4){
        demultiplex_usage();
        exit(-1);
     }
}

void demultiplex_usage(){

    fprintf(stderr, "\nUsage: atlas-utils demultiplex [options]\n\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -1 STR  forward reads file;\n");
    fprintf(stderr, "  -2 STR  reverse reads file;\n");
    fprintf(stderr, "  -b STR  barcode mapping file;\n");
    fprintf(stderr, "  -d STR  output filename directory;\n");
    fprintf(stderr, "  -l INT  minimum barcode length, default: 5;\n");
    fprintf(stderr, "  -s      don't delete barcode sequence;\n");
    fprintf(stderr, "  -h      display usage;\n");
    fprintf(stderr, "\nExample:\natlas-utils  demutiplex  -b barcodes.txt -1 I365.R1.fastq.gz  -2  I365.R2.fastq.gz -l 5  -d  I365\n\n");

    exit(-1);
}

void kh_demultiplex_destroy(khash_t(demultiplex) *h){
    
    khint_t k;
    if  (h == 0) return;
    for (k = 0; k < kh_end(h); ++k){
        if (kh_exist(h, k)){
            free( (char*)kh_key(h, k) );
            free( kh_val(h, k)->fs );
            free( kh_val(h, k)->rs );
            free( kh_val(h, k)->fn );
            free( kh_val(h, k)->rn );
            fclose( kh_val(h, k)->fh );
            fclose( kh_val(h, k)->rh );
        }
    }
    kh_destroy(demultiplex, h);
}

void kh_mem_destroy(khash_t(mem) *h){
    
    khint_t k;
    if  (h == 0) return;
    for (k = 0; k < kh_end(h); ++k){
        if (kh_exist(h, k)){
            free( (char*)kh_key(h, k) );
            int size = kh_val(h, k)->size;
            int i;

            for (i = 0; i < size; ++i) free(kh_val(h, k)->elem[i]);
            
            free(kh_val(h, k)->elem);
            free(kh_val(h, k)->mem.s);
            free(kh_val(h, k));
        }
    }
    kh_destroy(mem, h);
}

inline void demultiplex_print_seq(FILE *f, const kseq_t *s, int offset){

    fprintf(f, "@%s", s->name.s);
    if(s->comment.l) fprintf(f, " %s", s->comment.s);
    fprintf(f, "\n%s\n+\n%s\n", s->seq.s + offset, s->qual.s + offset); 

}

void demultiplex_make_fs(opt_t *opt){

    kstring_t kv = {0, 0, 0};
    ksprintf(&kv, "mkdir -p %s", opt->directory);
    if(  system(kv.s) != 0 ) exit (-1);
    free(kv.s);

}

void demultiplex_load_barcode( opt_t *opt,  khash_t(demultiplex) *h, khash_t(mem) *barcode_h) {

    gzFile fp;    
    fp = strcmp(opt->barcode, "-")? gzopen(opt->barcode, "r") : gzdopen(fileno(stdin), "r"); 

    if (fp) {

        kstream_t *ks;
        ks = ks_init(fp);
        kstring_t kt = {0, 0, 0};
        
        kstring_t kk = {0, 0, 0};
        kstring_t kv = {0, 0, 0};

        kstring_t linker = {0, 0, 0};
        
        int *fields, n, ret;
        khint_t k, t;

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

            FILE *fv = NULL;
            FILE *rv = NULL;
            
            Map *map  = (Map *) malloc( sizeof(Map) );
            map-> fs = strdup(kt.s + fields[1]);
            map-> rs = strdup(kt.s + fields[2]);
            map-> fl = strlen(kt.s + fields[1]);
            map-> rl = strlen(kt.s + fields[2]);
            map-> fn = strdup(kk.s);
            map-> rn = strdup(kv.s);
            map-> fh = fv;
            map-> rh = rv;

            k = kh_put(demultiplex, h, kt.s, &ret);
            kh_key(h, k)  = strdup( kt.s );
            kh_val(h, k)  = map;

            linker.l = 0;
            kputsn(kt.s + fields[1], opt->length, &linker);
            kputc('+', &linker);
            kputsn(kt.s + fields[2], opt->length, &linker);

            k = kh_get(mem, barcode_h, linker.s);
            if(k == kh_end(barcode_h)){
                
                t = kh_put(mem, barcode_h, linker.s , &ret);
                
                mem_t *mem = (mem_t *)malloc( sizeof(mem_t) );

                kh_key(barcode_h, t)  = strdup( linker.s );
                kh_val(barcode_h, t)  = mem;

                kstring_t ktmp        = {0, 0, 0};
                kputs(kt.s, &ktmp);
                kh_val(barcode_h, t)->mem  = ktmp;
                kh_val(barcode_h, t)->elem = NULL;
                kh_val(barcode_h, t)->size = 1;


            }else{             
            
                kputc('+',  &kh_val(barcode_h, k)->mem);
                kputs(kt.s, &kh_val(barcode_h, k)->mem);
                kh_val(barcode_h, k)->size ++;

            
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

void demultiplex( opt_t *opt,  khash_t(demultiplex) *h, khash_t(mem) *barcode_h ){


    gzFile fp[2];
    kseq_t *kseq[2];

    fp[0] = strcmp(opt->left, "-")? gzopen(opt->left, "r") : gzdopen(fileno(stdin), "r");
    kseq[0] = kseq_init(fp[0]);

    fp[1] = strcmp(opt->right, "-")? gzopen(opt->right, "r") : gzdopen(fileno(stdin), "r");
    kseq[1] = kseq_init(fp[1]);
    
    if (fp[0] == 0 || fp[1] == 0) {
       fprintf(stderr, "[ERR]: open file failed. check: %s|%s\n",  opt->left, opt->right);
       exit(-1);        
    }

    int *fields, i, n;

    khint_t  k,t;
    for (k = 0; k < kh_end(h); ++k){
      if (kh_exist(h, k)){
          kh_val(h, k)->fh = fopen(kh_val(h, k)->fn, "w");
          kh_val(h, k)->rh = fopen(kh_val(h, k)->rn, "w");
      }
    }

    kstring_t mem = {0, 0, 0};
    for (k = 0; k < kh_end(barcode_h); ++k){
      if (kh_exist(barcode_h, k)){
          
          int size = kh_val(barcode_h, k)->size;
          char **samples = (char**) malloc(sizeof(char *) * size);
          mem.l  = 0;
          kputs(kh_val(barcode_h, k)->mem.s, &mem); 
          fields = ksplit(&mem, '+', &n);
          for (i = 0; i < size; ++i) samples[i] = strdup(mem.s + fields[i]);
          kh_val(barcode_h, k)->elem  = samples;

      }
    }

    kstring_t linker = {0, 0, 0};
    kstring_t kk     = {0, 0, 0};
    kstring_t kv     = {0, 0, 0};
           
    while (kseq_read(kseq[0]) >= 0) {
        
        if (kseq_read(kseq[1]) < 0) {
            fprintf(stderr, "[W::%s]: the 2nd file has fewer records.\n", __func__);
            break;
        }

        //forwards
        
        linker.l = 0;
        kputsn(kseq[0]->seq.s, opt->length, &linker); kputc('+', &linker); kputsn(kseq[1]->seq.s, opt->length, &linker);

        k = kh_get(mem, barcode_h, linker.s);

        if( k != kh_end(barcode_h) ){

            for (i = 0; i < kh_val(barcode_h, k)->size; ++i){
               
               t = kh_get(demultiplex, h, kh_val(barcode_h, k)->elem[i]);

               if(t == kh_end(h)) {
                   fprintf(stderr, "[ERR]: barcode map error!\n");
                   exit(1);
               }else{

                    kk.l = kv.l = 0;
                    kputsn(kseq[0]->seq.s, kh_val(h, t)->fl , &kk);
                    kputsn(kseq[1]->seq.s, kh_val(h, t)->rl,  &kv);

                    if(strcmp(kk.s, kh_val(h, t)->fs) == 0 && strcmp(kv.s, kh_val(h, t)->rs) == 0 ){
                         
                         if(opt->split){
                            print_kseq(kseq[0], kh_val(h, t)->fh);
                            print_kseq(kseq[1], kh_val(h, t)->rh);
                         }else{
                            demultiplex_print_seq(kh_val(h, t)->fh, kseq[0], kh_val(h, t)->fl);
                            demultiplex_print_seq(kh_val(h, t)->rh, kseq[1], kh_val(h, t)->rl);
                         }
                    }
               }

            }
        }

        //reverse

        linker.l = 0;
        kputsn(kseq[1]->seq.s, opt->length, &linker); kputc('+', &linker); kputsn(kseq[0]->seq.s, opt->length, &linker);
        k = kh_get(mem, barcode_h, linker.s);
        if( k != kh_end(barcode_h) ){

            for (i = 0; i < kh_val(barcode_h, k)->size; ++i){
                   
               t = kh_get(demultiplex, h, kh_val(barcode_h, k)->elem[i]);
               if(t == kh_end(h)) {
                   fprintf(stderr, "barcode map error!\n");
                   exit(1);
               }else{
                    
                    kk.l = kv.l = 0;
                    kputsn(kseq[0]->seq.s, kh_val(h, t)->rl , &kk);
                    kputsn(kseq[1]->seq.s, kh_val(h, t)->fl,  &kv);

                    if(strcmp(kk.s, kh_val(h, t)->rs) == 0 && strcmp(kv.s, kh_val(h, t)->fs) == 0 ){
                        if(opt->split){
                          print_kseq(kseq[0], kh_val(h, t)->fh);
                          print_kseq(kseq[1], kh_val(h, t)->rh);
                        }else{
                          demultiplex_print_seq(kh_val(h, t)->fh, kseq[0], kh_val(h, t)->rl);
                          demultiplex_print_seq(kh_val(h, t)->rh, kseq[1], kh_val(h, t)->fl);
                        }
                    }
                }
            }
        }

    }
        
    if (kseq_read(kseq[1]) >= 0)
        fprintf(stderr, "[W::%s] the 1st file has fewer records.\n", __func__);
    
    if(kk.s != NULL ) free(kk.s);
    if(kv.s != NULL ) free(kv.s);
    if(linker.s != NULL ) free(linker.s);

    kseq_destroy(kseq[0]); gzclose(fp[0]);
    kseq_destroy(kseq[1]); gzclose(fp[1]);
 

}
