#include "utils.h"

void kh_int64_destroy(khash_t(int64) *h){
    
    khint_t k;
    if  (h == 0) return;
    for (k = 0; k < kh_end(h); ++k) {
        if (kh_exist(h, k)) free((int *)kh_val(h, k));
    }
    kh_destroy(int64, h);
}

void kh_int_destroy(khash_t(int) *h){
    
    khint_t k;
    if  (h == 0) return;
    for (k = 0; k < kh_end(h); ++k) {
        if (kh_exist(h, k)) {
            free((char*)kh_key(h, k));
        }
    }
    kh_destroy(int, h);
}

void kh_reg_destroy(khash_t(reg) *h){
    
    khint_t k;
    if  (h == 0) return;
    for (k = 0; k < kh_end(h); ++k) {
        if (kh_exist(h, k)) {
            free((char*)kh_key(h, k));
            free((char*)kh_val(h, k));
        }
    }
    kh_destroy(reg, h);

}

void kh_reg32_destroy(khash_t(reg32) *h){
    
    khint_t k;
    if  (h == 0) return;
    for (k = 0; k < kh_end(h); ++k) {
        if (kh_exist(h, k)) free((char*)kh_key(h, k));
    }
    kh_destroy(reg32, h);

}

void kh_kreg_destroy(khash_t(kreg) *h){
    
    khint_t k;
    if  (h == 0) return;
    for (k = 0; k < kh_end(h); ++k) {
        if (kh_exist(h, k)) {
            free((char*)kh_key(h, k));
            free((char*)kh_val(h, k).s);
        }
    }
    kh_destroy(kreg, h);

}

void kh_set_destroy(khash_t(set) *h){

    khint_t k;
    if (h == 0) return;
    for (k = 0; k < kh_end(h); ++k) {
        if (kh_exist(h, k)) {
            free((char*)kh_key(h, k));
        }
    }
    kh_destroy(set, h);
}

void kh_char_destroy(khash_t(char) *h){
    khint_t k;
    if (h == 0) return;
    for (k = 0; k < kh_end(h); ++k)
        if (kh_exist(h, k)) free( (char*)kh_val(h, k));
    kh_destroy(char, h);
}

void print_kseq(kseq_t *s, FILE *file){

    if( s->qual.l ){
        fprintf(file, "@%s", s->name.s);
        if (s->comment.l) fprintf(file, " %s", s->comment.s);
        fprintf(file, "\n%s\n+\n%s\n",s->seq.s, s->qual.s); 
    }else{
        fprintf(file, ">%s", s->name.s);
        if (s->comment.l) fprintf(file, " %s", s->comment.s);
        fprintf(file, "\n%s\n", s->seq.s); 
    }

}

void kh_double_destroy(khash_t(double) *h){
    
    khint_t k;
    if  (h == 0) return;
    for (k = 0; k < kh_end(h); ++k)
        if (kh_exist(h, k)) free((char*)kh_key(h, k));
    kh_destroy(double, h);
}

char comp_tab[] = {
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
     16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
     32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
     48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
     64, 'T', 'V', 'G', 'H', 'E', 'F', 'C', 'D', 'I', 'J', 'M', 'L', 'K', 'N', 'O',
    'P', 'Q', 'Y', 'S', 'A', 'A', 'B', 'W', 'X', 'R', 'Z',  91,  92,  93,  94,  95,
     64, 't', 'v', 'g', 'h', 'e', 'f', 'c', 'd', 'i', 'j', 'm', 'l', 'k', 'n', 'o',
    'p', 'q', 'y', 's', 'a', 'a', 'b', 'w', 'x', 'r', 'z', 123, 124, 125, 126, 127
};

void rev_com(kstring_t  *ks){
    
    int i, f, r;
    for (i = 0; i < ks->l>>1; ++i){
            f = comp_tab[(int)ks->s[i]];
            r = comp_tab[(int)ks->s[ks->l - 1 - i]];
            ks->s[i] = r;
            ks->s[ks->l - 1 - i] = f;
    }
    if (ks->l & 1) ks->s[ ks->l>>1 ] = comp_tab[(int)ks->s[ks->l>>1]];

}

void rev (kstring_t  *ks){
    
    int t, i;
    for (i = 0; i <= ks->l>>1; ++i){
            t = ks->s[i];
            ks->s[i] = ks->s[ks->l - 1 - i];
            ks->s[ks->l - 1 - i] = t;
    }

}

int get_target_column(kstring_t *kt, char *sample ){

    int *fields, n, i, col;
    col    = 0;
    fields = ksplit(kt, '\t', &n);
    for (i = 1; i < n; ++i)
        if(strcmp( kt->s + fields[i], sample ) == 0) col = i;

    if(col == 0){
         fprintf(stderr, "[ERR]: no such sample: %s\n" , sample );
         exit(-1);
    }
    return col;
}