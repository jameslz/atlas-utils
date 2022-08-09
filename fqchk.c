#include "utils.h"

static int qual;
int fqchk_main(int argc, char *argv[]){

    int  offset = 33;
    char *label ="Fastq";
    int  paired = 1;

    int c;
    while ((c = getopt(argc, argv, "q:l:p")) >= 0){
        if (c == 'q') offset = atoi(optarg);
        else if (c == 'l') label  = optarg;
        else if (c == 'p') paired = 2;
    }

    if ( optind == argc || argc != optind + 1) {
        fprintf(stderr, "\nUsage: atlas-utils fqchk [options] <in.fq>\n\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -q INT     offset value: 33 for Sanger quality, 64 for Illumina quality, default: [33]\n");
        fprintf(stderr, "  -p         specify input fastq as paired reads.\n");
        fprintf(stderr, "  -l STR     Fastq name\n\n");
        
        return 1;
    }

    int i, l = 0;

    int64_t lines   = 0;
    int64_t bases   = 0;
    int64_t q20     = 0;
    int64_t q30     = 0;

    gzFile fp;
    fp = strcmp(argv[optind], "-")? gzopen(argv[optind], "r") : gzdopen(fileno(stdin), "r");

    kseq_t *kseq;
    kseq = kseq_init(fp);

    while ((l = kseq_read(kseq)) >= 0){

        if(kseq->seq.l != kseq->qual.l){
            fprintf(stderr,"[ERR]: bad fastq file format!\n");
            exit(-1);
        }

        lines += 1;
        bases += kseq->seq.l;

        for (i = 0; i < kseq->seq.l; ++i){
            
            if(qual > 64 && offset != 64){
                fprintf(stderr, "\n[ERR]: need specify -q 64!\n");
                return 0;
            }
            qual = kseq->qual.s[i] - offset;
            if(qual >= 20) q20++;
            if(qual >= 30) q30++;
        }

    }
    
    kseq_destroy(kseq);
    printf("#sample\tsequence\tbase\tQ20(%%)\tQ30(%%)\n");
    printf("%s\t%lld\t%lld\t%.4lf\t%.4lf\n", label, (long long)lines/paired, (long long)bases,
                                             (100.0 * q20)/bases, (100.0 * q30)/bases);
    gzclose(fp);
    return 0;
}