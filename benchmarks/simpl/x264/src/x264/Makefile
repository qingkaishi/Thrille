# Makefile

include config.mak

all: default

SRCS = common/mc.c common/predict.c common/pixel.c common/macroblock.c \
       common/frame.c common/dct.c common/cpu.c common/cabac.c \
       common/common.c common/mdate.c common/set.c \
       common/quant.c common/vlc.c \
       encoder/analyse.c encoder/me.c encoder/ratecontrol.c \
       encoder/set.c encoder/macroblock.c encoder/cabac.c \
       encoder/cavlc.c encoder/encoder.c encoder/lookahead.c

SRCCLI = x264.c input/yuv.c input/y4m.c output/raw.c \
         output/matroska.c output/matroska_ebml.c

MUXERS := $(shell grep -E "(IN|OUT)PUT" config.h)

# Optional muxer module sources
ifneq ($(findstring AVIS_INPUT, $(MUXERS)),)
SRCCLI += input/avis.c
endif

ifneq ($(findstring HAVE_PTHREAD, $(CFLAGS)),)
SRCCLI += input/thread.c
endif

ifneq ($(findstring MP4_OUTPUT, $(MUXERS)),)
SRCCLI += output/mp4.c
endif

# Visualization sources
ifeq ($(VIS),yes)
SRCS   += common/visualize.c common/display-x11.c
endif

# MMX/SSE optims
ifneq ($(AS),)
X86SRC0 = cabac-a.asm dct-a.asm deblock-a.asm mc-a.asm mc-a2.asm \
          pixel-a.asm predict-a.asm quant-a.asm sad-a.asm \
          cpu-a.asm dct-32.asm
X86SRC = $(X86SRC0:%=common/x86/%)

ifeq ($(ARCH),X86)
ARCH_X86 = yes
ASMSRC   = $(X86SRC) common/x86/pixel-32.asm
endif

ifeq ($(ARCH),X86_64)
ARCH_X86 = yes
ASMSRC   = $(X86SRC:-32.asm=-64.asm)
ASFLAGS += -DARCH_X86_64
endif

ifdef ARCH_X86
ASFLAGS += -Icommon/x86/
SRCS   += common/x86/mc-c.c common/x86/predict-c.c
OBJASM  = $(ASMSRC:%.asm=%.o)
$(OBJASM): common/x86/x86inc.asm common/x86/x86util.asm
checkasm: tools/checkasm-a.o
endif
endif

# AltiVec optims
ifeq ($(ARCH),PPC)
SRCS += common/ppc/mc.c common/ppc/pixel.c common/ppc/dct.c \
        common/ppc/quant.c common/ppc/deblock.c \
        common/ppc/predict.c
endif

# NEON optims
ifeq ($(ARCH),ARM)
ifneq ($(AS),)
ASMSRC += common/arm/cpu-a.S common/arm/pixel-a.S common/arm/mc-a.S \
          common/arm/dct-a.S common/arm/quant-a.S common/arm/deblock-a.S \
          common/arm/predict-a.S
SRCS   += common/arm/mc-c.c common/arm/predict-c.c
OBJASM  = $(ASMSRC:%.S=%.o)
endif
endif

# VIS optims
ifeq ($(ARCH),UltraSparc)
ASMSRC += common/sparc/pixel.asm
OBJASM  = $(ASMSRC:%.asm=%.o)
endif

ifneq ($(HAVE_GETOPT_LONG),1)
SRCS += extras/getopt.c
endif

OBJS = $(SRCS:%.c=%.o)
OBJCLI = $(SRCCLI:%.c=%.o)
DEP  = depend

.PHONY: all default fprofiled clean distclean install uninstall dox test testclean

default: $(DEP) x264$(EXE)

libx264.a: .depend $(OBJS) $(OBJASM)
	$(AR) rc libx264.a $(OBJS) $(OBJASM)
	$(RANLIB) libx264.a

$(SONAME): .depend $(OBJS) $(OBJASM)
	$(CC) -shared -o $@ $(OBJS) $(OBJASM) $(SOFLAGS) $(LDFLAGS)

x264$(EXE): $(OBJCLI) libx264.a 
	$(CC) -o $@ $+ $(LDFLAGS) -L$(THRILLE_ROOT)/bin/ -ldummy

checkasm: tools/checkasm.o libx264.a
	$(CC) -o $@ $+ $(LDFLAGS)

%.o: %.asm
	$(AS) $(ASFLAGS) -o $@ $<
	-@ $(STRIP) -x $@ # delete local/anonymous symbols, so they don't show up in oprofile

%.o: %.S
	$(AS) $(ASFLAGS) -o $@ $<
	-@ $(STRIP) -x $@ # delete local/anonymous symbols, so they don't show up in oprofile

.depend: config.mak
	rm -f .depend
	$(foreach SRC, $(SRCS) $(SRCCLI), $(CC) $(CFLAGS) $(ALTIVECFLAGS) $(SRC) -MT $(SRC:%.c=%.o) -MM -g0 1>> .depend;)

config.mak:
	./configure

depend: .depend
ifneq ($(wildcard .depend),)
include .depend
endif

SRC2 = $(SRCS) $(SRCCLI)
# These should cover most of the important codepaths
OPT0 = --crf 30 -b1 -m1 -r1 --me dia --no-cabac --direct temporal --ssim --no-weightb
OPT1 = --crf 16 -b2 -m3 -r3 --me hex --no-8x8dct --direct spatial --no-dct-decimate -t0  --slice-max-mbs 50
OPT2 = --crf 26 -b4 -m5 -r2 --me hex --cqm jvt --nr 100 --psnr --no-mixed-refs --b-adapt 2 --slice-max-size 1500
OPT3 = --crf 18 -b3 -m9 -r5 --me umh -t1 -A all --b-pyramid normal --direct auto --no-fast-pskip --no-mbtree
OPT4 = --crf 22 -b3 -m7 -r4 --me esa -t2 -A all --psy-rd 1.0:1.0 --slices 4
OPT5 = --frames 50 --crf 24 -b3 -m10 -r3 --me tesa -t2
OPT6 = --frames 50 -q0 -m9 -r2 --me hex -Aall
OPT7 = --frames 50 -q0 -m2 -r1 --me hex --no-cabac

ifeq (,$(VIDS))
fprofiled:
	@echo 'usage: make fprofiled VIDS="infile1 infile2 ..."'
	@echo 'where infiles are anything that x264 understands,'
	@echo 'i.e. YUV with resolution in the filename, y4m, or avisynth.'
else
fprofiled:
	$(MAKE) clean
	mv config.mak config.mak2
	sed -e 's/CFLAGS.*/& -fprofile-generate/; s/LDFLAGS.*/& -fprofile-generate/' config.mak2 > config.mak
	$(MAKE) x264$(EXE)
	$(foreach V, $(VIDS), $(foreach I, 0 1 2 3 4 5 6 7, ./x264$(EXE) $(OPT$I) --threads 1 $(V) -o $(DEVNULL) ;))
	rm -f $(SRC2:%.c=%.o)
	sed -e 's/CFLAGS.*/& -fprofile-use/; s/LDFLAGS.*/& -fprofile-use/' config.mak2 > config.mak
	$(MAKE)
	rm -f $(SRC2:%.c=%.gcda) $(SRC2:%.c=%.gcno)
	mv config.mak2 config.mak
endif

clean:
	rm -f $(OBJS) $(OBJASM) $(OBJCLI) $(SONAME) *.a x264 x264.exe .depend TAGS
	rm -f checkasm checkasm.exe tools/checkasm.o tools/checkasm-a.o
	rm -f $(SRC2:%.c=%.gcda) $(SRC2:%.c=%.gcno)
	- sed -e 's/ *-fprofile-\(generate\|use\)//g' config.mak > config.mak2 && mv config.mak2 config.mak

distclean: clean
	rm -f config.mak config.h x264.pc
	rm -rf test/

install: x264$(EXE) $(SONAME)
	install -d $(DESTDIR)$(bindir) $(DESTDIR)$(includedir)
	install -d $(DESTDIR)$(libdir) $(DESTDIR)$(libdir)/pkgconfig
	install -m 644 x264.h $(DESTDIR)$(includedir)
	install -m 644 libx264.a $(DESTDIR)$(libdir)
	install -m 644 x264.pc $(DESTDIR)$(libdir)/pkgconfig
	install x264$(EXE) $(DESTDIR)$(bindir)
	$(RANLIB) $(DESTDIR)$(libdir)/libx264.a
ifeq ($(SYS),MINGW)
	$(if $(SONAME), install -m 755 $(SONAME) $(DESTDIR)$(bindir))
else
	$(if $(SONAME), ln -sf $(SONAME) $(DESTDIR)$(libdir)/libx264.$(SOSUFFIX))
	$(if $(SONAME), install -m 755 $(SONAME) $(DESTDIR)$(libdir))
endif
	$(if $(IMPLIBNAME), install -m 644 $(IMPLIBNAME) $(DESTDIR)$(libdir))

uninstall:
	rm -f $(DESTDIR)$(includedir)/x264.h $(DESTDIR)$(libdir)/libx264.a
	rm -f $(DESTDIR)$(bindir)/x264 $(DESTDIR)$(libdir)/pkgconfig/x264.pc
	$(if $(SONAME), rm -f $(DESTDIR)$(libdir)/$(SONAME) $(DESTDIR)$(libdir)/libx264.$(SOSUFFIX))

etags: TAGS

TAGS:
	etags $(SRCS)

dox:
	doxygen Doxyfile

ifeq (,$(VIDS))
test:
	@echo 'usage: make test VIDS="infile1 infile2 ..."'
	@echo 'where infiles are anything that x264 understands,'
	@echo 'i.e. YUV with resolution in the filename, y4m, or avisynth.'
else
test:
	perl tools/regression-test.pl --version=head,current --options='$(OPT0)' --options='$(OPT1)' --options='$(OPT2)' $(VIDS:%=--input=%)
endif

testclean:
	rm -f test/*.log test/*.264
	$(foreach DIR, $(wildcard test/x264-r*/), cd $(DIR) ; make clean ; cd ../.. ;)
