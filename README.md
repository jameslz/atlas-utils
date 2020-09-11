# atlas-utils: tools for microbiome data analysis

### 1、Introduction

    atlas-utils: tools for microbiome data analysis, using klib.

### 2、 interface

current: `version：0.0.1-r2`

    $ atlas-utils
    
    Usage:   atlas-utils <command> <arguments>
    Version: 0.0.1 r2
    
    Command:
          -- Fastq manipulation.
             demultiplex       FASTAQ demultiplex with barcode sequence.
             split             FASTAQ demultiplex with barcode without
                               trimming barcode sequence.
             uniques           Find unique sequences and abundances.
             linkpairs         Link paired-reads with padding 'N'.
             fqchk             Fastq QC (base/quality summary).
             label             Add label to fasta headline.
             barcode           Get barcode seq from paired-reads.
             orient            Orient seq use strand +/- information.
             primer_strip      Retrive sequence region matches to primer pair.
                               support single end match.
    
          -- (Z)OTU manipulation.
             unique_table     Construct uniques table using exact match.
             annotation       Annotate OTU table using OTU assignment from Sintax/RDP.
             filter           1. Filter sequence from Mitochondria/Chloroplast, 2. optional filter
                              specifed taxon level .
             level            Abundance table for specify taxonomy level. ie. genus.
             abundance        Abundance table for specify taxonomy level and sample. ie. genus.
             summary          Calcuate OTU number/Tag number per sample.
             rowsum           Calcuate row sum.
             rank             Rank/merge for numeric table.
             krona            Convert annotated OTU table to krona text format.
             quantile         Calcuate  nth quantile sample size for otu_table normalization.
             mean_size        Calcuate 'Mean' sample size for otu_table normalization.
             min_size         Calcuate 'Min' sample size for otu_table normalization.
             max_size         Calcuate 'Max' sample size for otu_table normalization.
             counts2freqs     Convert counts table from counts to frequencies.
             trim             Delete low-abundance (Z)OTUs.
             binary           Convert (Z)OTUs table to presence/absence values.
             pairwise         Convert (Z)OTUs table to pairwise format for phylocom.
             rarity           rarity summary.
             rare             Rarefy OTU table to specified number of reads.
             group_by         group_by operation for OTU table, support average, sum.
    
          -- PICRUSt.
             normalization    Normalize OTU table with 16S copy Number, et..
             melt             Calculate feature abundance. ie. KEGG, COG, with Greengene 13.5.
             kann             Calculate feature abundance using feature maps, KO -> Module.
    
          -- auxiliary utils.
             view             View text file, ignor comments and blank lines.
             getline          Get target line with headline.
             subsamples       Get target columns with headline match.
             partition        Split OTU table into specify number file.

    Licenced:
    (c) 2019-2020 - LEI ZHANG
    Logic Informatics Co.,Ltd.
    zhanglei@logicinformatics.com
