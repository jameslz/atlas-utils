CC=		cc
CFLAGS=		-g -Wall -O2 -Wno-unused-function
CPPFLAGS=
INCLUDES=   -I libxlsxwriter/include
PROG=		atlas-utils
BINDIR=		/usr/local/bin
LIBS=		-lz -lm libxlsxwriter/lib/libxlsxwriter.a
OBJS=		demultiplex.o trim.o linkpairs.o fqchk.o label.o barcode.o orient.o primer_strip.o  maxee.o \
			annotation.o unique_table.o filter.o abundance.o level.o summary.o rowsum.o krona.o quantile.o  mean_size.o \
			min_size.o max_size.o counts2freqs.o rank.o binary.o pairwise.o rarity.o rare.o group_by.o  uniques.o \
			normalization.o melt.o kann.o view.o getline.o subsamples.o partition.o patch.o flatten.o unpack.o lefse.o \
			subgroup.o hierarchy.o core.o dt2xlsx.o prune.o cv.o fish.o stack.o strip.o aggregate.o contrib.o \
			fastrand.o kstring.o utils.o

.SUFFIXES:.c .o
.PHONY:all clean

.c.o:
		$(CC) -c $(CFLAGS) $(CPPFLAGS) $(INCLUDES) $< -o $@

all:$(PROG)

atlas-utils:$(OBJS) atlas-utils.o
		$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

install:all
		install atlas-utils $(BINDIR)

clean:
		rm -fr *.o atlas-utils