###  atlas-utils: tools for microbiome data analysis
<hr>

#### 1. install

```sh
git clone https://github.com/jameslz/atlas-utils
cd atlas-utils
make
```

#### 2、 interface

current: `version：0.0.2`


```text
    Usage:   atlas-utils <command> <arguments>
    Version: 0.0.2

    Command:
          -- Fastq manipulation.
             demultiplex       FASTAQ demultiplex with barcode sequence.
             uniques           find unique sequences and abundances.
             linkpairs         link paired-reads with padding 'N'.
             fqchk             fastq QC (base/quality summary).
             label             add label to fasta headline.
             barcode           get barcode seq from paired-reads.
             orient            orient seq use strand +/- information.
             maxee             qualtiy control with quality error.
             primer_strip      retrive sequence region matches to primer pair.
                               support single end match.

          -- (Z)OTU manipulation.
             unique_table     construct uniques table using exact match.
             annotation       annotate OTU table using OTU assignment from Sintax/RDP.
             filter           1. Filter sequence from Mitochondria/Chloroplast, 2. optional filter
                              specifed taxon level .
             level            abundance table for specify taxonomy level. ie. genus.
             abundance        abundance table for specify taxonomy level and sample. ie. genus.
             summary          calcuate OTU number/Tag number per sample.
             rowsum           calcuate row sum.
             rank             rank/merge for numeric table.
             krona            convert annotated OTU table to krona text format.
             quantile         calcuate  nth quantile sample size for otu_table normalization.
             mean_size        calcuate 'Mean' sample size for otu_table normalization.
             min_size         calcuate 'Min' sample size for otu_table normalization.
             max_size         calcuate 'Max' sample size for otu_table normalization.
             counts2freqs     convert counts table from counts to frequencies.
             trim             delete low-abundance (Z)OTUs with sum threshold.
             prune            delete low-abundance (Z)OTUs.with per sample threshold
             binary           convert (Z)OTUs table to presence/absence values.
             pairwise         convert (Z)OTUs table to pairwise format for phylocom.
             rarity           rarity summary.
             rare             rarefy OTU table to specified number of reads.
             core             core microbiome per/group.
             group_by         group_by operation for OTU table, support average, sum.
             flatten          flatten the zotu table taxonomy annotation.
             unpack           unpack the sintax classify result.
             lefse            convert to lefse compatibility format.
             subgroup         retrieve columns in in list of columns.
             hierarchy        hierarchy format for classic annotated OTU table.
             cv               calcuate C.V|Coefficient of Variation.
             fish             select OTUs using specifed abundance threshold.
             stack            reshape OTU table for data visulization.

          -- PICRUSt.
             normalization    normalize OTU table with 16S copy Number, et..
             aggregate        merge same hits with sum opt.
             contrib          calculate feature abundance contribution.
             melt             calculate feature abundance. ie. KEGG, COG, with Greengene 13.5.
             kann             calculate feature abundance using feature maps, KO -> Module.

          -- auxiliary utils.
             view             view text file, ignor comments and blank lines.
             getline          get target line with headline.
             subsamples       get target columns with headline match.
             partition        split OTU table into specify number file.
             dt2xlsx          convert data table (multi-) file to Excel file..
             strip            strip any char from first space in first columns.


    Licenced:
    (c) 2019-2022 - LEI ZHANG
    Logic Informatics Co.,Ltd.
    zhanglei@logicinformatics.com
```