#include "utils.h"

#include <math.h>
#include <float.h>

static double *vt = NULL; 

double cv(int n){

    double sum = 0., avg = 0.;
    
    int i;
    for (i = 0; i < n; ++i) sum += vt[i];
    avg = sum / n;

    double sum2 = 0.;
    for (i = 0; i < n; ++i) {
        double t = (vt[i] - avg) * (vt[i] - avg);
        sum2 += t;
    }
    return sqrt(sum2 / n) / avg;

}

int cv_main (int argc, char *argv[]){
    
    if ( optind == argc || argc != optind + 1) {
        
        fprintf(stderr, "\nUsage: atlas-utils cv  <tsv>\n\n");
        return 1;
    
    }
    
    printf("#id\tcv\n");

    gzFile     fp;
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r");

    kstream_t *ks;
    ks = ks_init(fp);

    if (fp) {

        kstring_t  kt  = {0, 0, 0};
        ks = ks_init(fp);
        int *fields, i, n;

        if( ks_getuntil(ks, '\n', &kt, 0) >=  0 ){

            if(kt.s[0] == '#'){
                fields  = ksplit(&kt, '\t', &n);
                vt = (double *)calloc(n - 1, sizeof(double));
            }
            else{
                fprintf(stderr, "[ERR]: first line not start with '#' \n %s\n", kt.s);
                exit(-1);
            }

        }

        while( ks_getuntil(ks, '\n', &kt, 0) >=  0 ){
            
            fields  = ksplit(&kt, '\t', &n);
            if(n == 2){
               printf("%s\t%lf\n", kt.s, 0.);
               continue; 
            }

            for (i = 1; i < n; ++i) vt[i - 1] = atof( kt.s + fields[i] );
            printf("%s\t%lf\n", kt.s, cv(n - 1));
            
        }
        free(kt.s);
        ks_destroy(ks);
        gzclose(fp);

    }else{
        fprintf(stderr, "[ERR]: can't open file %s\n", argv[optind]);
        exit(1);
    }

    if(vt != NULL) free(vt);
    return 0;
}