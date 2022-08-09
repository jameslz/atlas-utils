#include "utils.h"
#include <math.h>

#include "ksort.h"
KSORT_INIT_GENERIC(double)

double inline quantile (double *x, double q, const int size){

    if (q >= 1.0f)  return x[size - 1];
    if( q <= 0.0f)  return x[0];
    return x[ (int)(ceil( size * q) -  1) ];

}

int quantile_main (int argc, char *argv[]){
    
    if ( argc != 3) {
        fprintf(stderr, "\nUsage: atlas-utils quantile <otutab> <quantile|q, ie: 0.95>\n\n");
        return 1;
    }

    int size = 0;
    double *buf;

    gzFile fp;
    fp = strcmp(argv[1], "-")? gzopen(argv[1], "r") : gzdopen(fileno(stdin), "r");
    if (fp) {

        kstring_t kt = {0, 0, 0};
        int *fields, n, i;
        kstream_t *ks;
        ks = ks_init(fp);

        if( ks_getuntil( ks, '\n', &kt, 0) >=  0 ){
            
            if( kt.s[0] == '#'){
                ksplit(&kt, '\t', &size);
                size = size - 1;
            }else{
                fprintf(stderr, "[ERR]: no title fields, exit.\n");
                exit(-1);
            }
        }

        buf = (double *) calloc(size, sizeof(double));

        while( ks_getuntil( ks, '\n', &kt, 0) >=  0){

            fields = ksplit(&kt,'\t', &n);
            for (i = 1; i < n; ++i) buf[i -1] +=  atof(kt.s + fields[i]);     
        
        }

    	free(kt.s);
        ks_destroy(ks);
        gzclose(fp);

    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[1]);
        exit(1);
    }

    ks_mergesort(double, size, buf,  0);

    printf("%.2lf\n", quantile(buf, atof(argv[2]), size));

    if(buf != NULL) free(buf);
    return 0;

}