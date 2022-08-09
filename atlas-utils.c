/*****************************************************************************
  atlas-utils.c

  atlas-utils command line interface.  

  (c) 2019-2022 - LEI ZHANG
  Logic Informatics Co.,Ltd.
  zhanglei@logicinformatics.com
  
  Licenced under The MIT License.
******************************************************************************/

#include <stdio.h>
#include <string.h>

int demultiplex_main(int argc, char *argv[]);
int trim_main(int argc, char *argv[]);
int linkpairs_main(int argc, char *argv[]);
int fqchk_main(int argc, char *argv[]);
int label_main(int argc, char *argv[]);
int barcode_main(int argc, char *argv[]);
int orient_main(int argc, char *argv[]);
int primer_strip_main(int argc, char *argv[]);
int maxee_main(int argc, char *argv[]);

int annotation_main(int argc, char *argv[]);

int unique_table_main(int argc, char *argv[]);
int filter_main(int argc, char *argv[]);
int abundance_main(int argc, char *argv[]);
int level_main(int argc, char *argv[]);
int summary_main(int argc, char *argv[]);
int rowsum_main(int argc, char *argv[]);
int krona_main(int argc, char *argv[]);
int quantile_main(int argc, char *argv[]);
int mean_size_main(int argc, char *argv[]);
int min_size_main(int argc, char *argv[]);
int max_size_main(int argc, char *argv[]);
int counts2freqs_main(int argc, char *argv[]);
int rank_main(int argc, char *argv[]);
int binary_main(int argc, char *argv[]);
int pairwise_main(int argc, char *argv[]);
int rarity_main(int argc, char *argv[]);
int rare_main(int argc, char *argv[]);
int group_by_main(int argc, char *argv[]);

int uniques_main(int argc, char *argv[]);

int normalization_main(int argc, char *argv[]);
int melt_main(int argc, char *argv[]);
int kann_main(int argc, char *argv[]);

int view_main(int argc, char *argv[]);
int getline_main(int argc, char *argv[]);
int subsamples_main(int argc, char *argv[]);
int partition_main(int argc, char *argv[]);
int patch_main(int argc, char *argv[]);
int flatten_main(int argc, char *argv[]);
int unpack_main(int argc, char *argv[]);
int lefse_main(int argc, char *argv[]);
int subgroup_main(int argc, char *argv[]);
int hierarchy_main(int argc, char *argv[]);
int core_main(int argc, char *argv[]);
int dt2xlsx_main(int argc, char *argv[]);
int prune_main(int argc, char *argv[]);
int cv_main(int argc, char *argv[]);
int fish_main(int argc, char *argv[]);
int stack_main(int argc, char *argv[]);
int strip_main(int argc, char *argv[]);
int aggregate_main(int argc, char *argv[]);
int contrib_main(int argc, char *argv[]);

static int usage(){
    
    fprintf(stderr, "\n");
    fprintf(stderr, "Usage:   atlas-utils <command> <arguments>\n");
    fprintf(stderr, "Version: 0.0.2\n\n");
    fprintf(stderr, "Command:\n");

    fprintf(stderr, "      -- Fastq manipulation.\n");
    fprintf(stderr, "         demultiplex       FASTAQ demultiplex with barcode sequence.\n");
    fprintf(stderr, "         uniques           find unique sequences and abundances.\n");
    fprintf(stderr, "         linkpairs         link paired-reads with padding 'N'.\n");
    fprintf(stderr, "         fqchk             fastq QC (base/quality summary).\n");
    fprintf(stderr, "         label             add label to fasta headline.\n");
    fprintf(stderr, "         barcode           get barcode seq from paired-reads.\n");
    fprintf(stderr, "         orient            orient seq use strand +/- information.\n");
    fprintf(stderr, "         maxee             qualtiy control with quality error.\n");
    fprintf(stderr, "         primer_strip      retrive sequence region matches to primer pair.\n");
    fprintf(stderr, "                           support single end match.\n");

    fprintf(stderr, "\n");

    fprintf(stderr, "      -- (Z)OTU manipulation.\n");
    fprintf(stderr, "         unique_table     construct uniques table using exact match.\n");
    fprintf(stderr, "         annotation       annotate OTU table using OTU assignment from Sintax/RDP.\n");
    fprintf(stderr, "         filter           1. Filter sequence from Mitochondria/Chloroplast, 2. optional filter\n");
    fprintf(stderr, "                          specifed taxon level .\n");
    fprintf(stderr, "         level            abundance table for specify taxonomy level. ie. genus.\n");
    fprintf(stderr, "         abundance        abundance table for specify taxonomy level and sample. ie. genus.\n");
    fprintf(stderr, "         summary          calcuate OTU number/Tag number per sample.\n");
    fprintf(stderr, "         rowsum           calcuate row sum.\n");
    fprintf(stderr, "         rank             rank/merge for numeric table.\n");
    fprintf(stderr, "         krona            convert annotated OTU table to krona text format.\n");
    fprintf(stderr, "         quantile         calcuate  nth quantile sample size for otu_table normalization.\n");
    fprintf(stderr, "         mean_size        calcuate 'Mean' sample size for otu_table normalization.\n");
    fprintf(stderr, "         min_size         calcuate 'Min' sample size for otu_table normalization.\n");
    fprintf(stderr, "         max_size         calcuate 'Max' sample size for otu_table normalization.\n");
    fprintf(stderr, "         counts2freqs     convert counts table from counts to frequencies.\n");
    fprintf(stderr, "         trim             delete low-abundance (Z)OTUs with sum threshold.\n");
    fprintf(stderr, "         prune            delete low-abundance (Z)OTUs.with per sample threshold\n");
    fprintf(stderr, "         binary           convert (Z)OTUs table to presence/absence values.\n");
    fprintf(stderr, "         pairwise         convert (Z)OTUs table to pairwise format for phylocom.\n");
    fprintf(stderr, "         rarity           rarity summary.\n");
    fprintf(stderr, "         rare             rarefy OTU table to specified number of reads.\n");
    fprintf(stderr, "         core             core microbiome per/group.\n");
    fprintf(stderr, "         group_by         group_by operation for OTU table, support average, sum.\n");
    fprintf(stderr, "         flatten          flatten the zotu table taxonomy annotation.\n");
    fprintf(stderr, "         unpack           unpack the sintax classify result.\n");
    fprintf(stderr, "         lefse            convert to lefse compatibility format.\n");
    fprintf(stderr, "         subgroup         retrieve columns in in list of columns.\n");
    fprintf(stderr, "         hierarchy        hierarchy format for classic annotated OTU table.\n");
    fprintf(stderr, "         cv               calcuate C.V|Coefficient of Variation.\n");
    fprintf(stderr, "         fish             select OTUs using specifed abundance threshold.\n");
    fprintf(stderr, "         stack            reshape OTU table for data visulization.\n");

    fprintf(stderr, "\n");

    fprintf(stderr, "      -- PICRUSt.\n");
    fprintf(stderr, "         normalization    normalize OTU table with 16S copy Number, et..\n");
    fprintf(stderr, "         aggregate        merge same hits with sum opt.\n");
    fprintf(stderr, "         contrib          calculate feature abundance contribution.\n");
    fprintf(stderr, "         melt             calculate feature abundance. ie. KEGG, COG, with Greengene 13.5.\n");
    fprintf(stderr, "         kann             calculate feature abundance using feature maps, KO -> Module.\n");
    fprintf(stderr, "\n");


    fprintf(stderr, "      -- auxiliary utils.\n");
    fprintf(stderr, "         view             view text file, ignor comments and blank lines.\n");
    fprintf(stderr, "         getline          get target line with headline.\n");
    fprintf(stderr, "         subsamples       get target columns with headline match.\n");
    fprintf(stderr, "         partition        split OTU table into specify number file.\n");
    fprintf(stderr, "         dt2xlsx          convert data table (multi-) file to Excel file..\n");
    fprintf(stderr, "         strip            strip any char from first space in first columns.\n");

    fprintf(stderr, "\n");

    fprintf(stderr, "\nLicenced:\n");
    fprintf(stderr, "(c) 2019-2022 - LEI ZHANG\n");
    fprintf(stderr, "Logic Informatics Co.,Ltd.\n");
    fprintf(stderr, "zhanglei@logicinformatics.com\n");
    fprintf(stderr, "\n");
    
    return 1;

}

int main(int argc, char *argv[]){

    if (argc < 2) return usage();

    if (strcmp(argv[1], "trim") == 0)                  trim_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "demultiplex") == 0)      demultiplex_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "fqchk") == 0)            fqchk_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "label") == 0)            label_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "linkpairs") == 0)        linkpairs_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "orient") == 0)           orient_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "uniques") == 0)          uniques_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "maxee") == 0)            maxee_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "barcode") == 0)          barcode_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "unique_table") == 0)     unique_table_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "annotation") == 0)       annotation_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "filter") == 0)           filter_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "abundance") == 0)        abundance_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "level") == 0)            level_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "summary") == 0)          summary_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "group_by") == 0)         group_by_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "rowsum") == 0)           rowsum_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "rank") == 0)             rank_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "binary") == 0)           binary_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "pairwise") == 0)         pairwise_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "rare") == 0)             rare_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "krona") == 0)            krona_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "quantile") == 0)         quantile_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "mean_size") == 0)        mean_size_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "min_size") == 0)         min_size_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "max_size") == 0)         max_size_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "counts2freqs") == 0)     counts2freqs_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "primer_strip") == 0)     primer_strip_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "view") == 0)             view_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "kann") == 0)             kann_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "normalization") == 0)    normalization_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "melt") == 0)             melt_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "getline") == 0)          getline_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "subsamples") == 0)       subsamples_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "partition") == 0)        partition_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "rarity") == 0)           rarity_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "patch") == 0)            patch_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "flatten") == 0)          flatten_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "lefse") == 0)            lefse_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "unpack") == 0)           unpack_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "prune") == 0)            prune_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "subgroup") == 0)         subgroup_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "hierarchy") == 0)        hierarchy_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "dt2xlsx") == 0)          dt2xlsx_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "core") == 0)             core_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "cv") == 0)               cv_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "fish") == 0)             fish_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "stack") == 0)            stack_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "strip") == 0)            strip_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "aggregate") == 0)        aggregate_main(argc - 1, argv + 1);
    else if (strcmp(argv[1], "contrib") == 0)          contrib_main(argc - 1, argv + 1);
    else {
        fprintf(stderr, "[main] unrecognized command '%s'. Abort!\n", argv[1]);
        return 1;
    }
    return 0;

}