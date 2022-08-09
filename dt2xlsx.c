#include "utils.h"
#include "xlsxwriter.h"

int dt2xlsx_main(int argc, char *argv[]){
    
    if ( argc < 3 ) {
        fprintf(stderr, "\n");
        fprintf(stderr, "Usage: tsv-utils dt2xlsx <xlsx> [sheet-name:file_path ...]\n\n");
        return 1;
    }

    kstream_t *ks;
    kstring_t kt = {0, 0, 0};
 
    lxw_workbook  *workbook  = workbook_new(argv[1]);

    gzFile     fp;
    int i;
    for (i = 2; i < argc; ++i){

         ks_tokaux_t aux;  char *p;      
         p = kstrtok(argv[i], ":", &aux);
         argv[i][ aux.p - p] = '\0';
         
         fp = gzopen(aux.p + 1, "r");
         if(fp){

             int row = 0;             
             ks = ks_init(fp);
             lxw_worksheet *worksheet = workbook_add_worksheet(workbook, p);
             char *p;

             if( ks_getuntil( ks, '\n', &kt, 0) >=  0){
                  int k;
                  ks_tokaux_t aux;
                  for (p = kstrtok(kt.s, "\t", &aux), k = 0; p; p = kstrtok(0, 0, &aux), ++k){
                     kt.s[aux.p - kt.s] = '\0';
                     worksheet_write_string(worksheet, row, k, p , NULL);  
                  }

                  ++row;
             }

             while( ks_getuntil( ks, '\n', &kt, 0) >=  0){
                  int k;
                  ks_tokaux_t aux;
                  for (p = kstrtok(kt.s, "\t", &aux), k = 0; p; p = kstrtok(0, 0, &aux), ++k){
                     kt.s[aux.p - kt.s] = '\0';
                     
                     if(k == 0)
                         worksheet_write_string(worksheet, row, k, p , NULL);
                     else
                         worksheet_write_number(worksheet, row, k, atof(p) , NULL);
          
                  }

                  ++row;
             }

             ks_destroy(ks);
             gzclose(fp);

         }else{
             fprintf(stderr, "[ERR]: can't open %s!\n", aux.p);
         }
    }

    free(kt.s);
    return workbook_close(workbook);

}