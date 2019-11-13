#
# Build Scott19
# 

SDCCDIR = i:/sdcc
CC = sdcc
AS = sdasz80
LD = sdldz80
DEBUG =
GCC = gcc

# callee-saves appears not available on Z80
OPTBASE = --opt-code-size #--all-callee-saves

OPT = $(OPTBASE) --max-allocs-per-node 20000
OPT2 = $(OPTBASE) --max-allocs-per-node 100000 

ASFLAGS = -l
DEFS = -DSMALL -DNDEBUG 

CFLAGS = -mz80 --std-sdcc11 --fsigned-char $(OPT) $(DEBUG) $(DEFS)


## set up your path to where the SDCC Z80 lib is
LIBS = -l $(SDCCDIR)/lib/z80/z80.lib


# DOS machine
LDFLAGS = -mjwx -b _CODE=0x5200 $(LIBS)

OBJS = \
	crt0.rel \
	os.rel \
	utils.rel \
	scott.rel \
	scottfree.rel \
	sglue.rel \
	fios.rel

%.rel: %.c
	$(CC) $(CFLAGS) -c $< 

%.rel: %.s
	$(AS) $(ASFLAGS) -o $@ $<

%.dat.h: %.dat dat2c
	./dat2c $<


all: adv05.dsk

# make scott
# will build host (pc) version of scott
scott: sgluehost.c scottfree.c utils.c scottfree.h sglue.h
	$(GCC) -Wall -DDAT_FILES -o scott sgluehost.c scottfree.c utils.c

dat2c: dat2c.c scottfree.c utils.c scottfree.h sglue.h
	$(GCC) -DDAT_FILES -o dat2c dat2c.c scottfree.c utils.c

%.cas: %.ihx
	../tools/mksys/mksys $< $@


adv01.ihx : DEFS += -DGAME="adv01.dat.h"
adv01.ihx : adv01.dat.h $(OBJS) Makefile
	$(LD) $(LDFLAGS) -i $@ $(OBJS)

adv02.ihx : DEFS += -DGAME="adv02.dat.h"
adv02.ihx : adv02.dat.h $(OBJS) Makefile
	$(LD) $(LDFLAGS) -i $@ $(OBJS)

adv03.ihx : DEFS += -DGAME="adv03.dat.h"
adv03.ihx : adv03.dat.h $(OBJS) Makefile
	$(LD) $(LDFLAGS) -i $@ $(OBJS)

adv04.ihx : DEFS += -DGAME="adv04.dat.h"
adv04.ihx : adv04.dat.h $(OBJS) Makefile
	$(LD) $(LDFLAGS) -i $@ $(OBJS)

adv05.ihx : DEFS += -DGAME="adv05.dat.h"
adv05.ihx : adv05.dat.h $(OBJS) Makefile
	$(LD) $(LDFLAGS) -i $@ $(OBJS)


%.cmd: %.cas
	../tools/trld/trld $< $@

%.dsk: %.cmd
	cp ../emu/blank.dsk $@
	../tools/trswrite -o $@ $<

.PHONY:	 clean cleanall tags

clean:
	rm -f *.rel
	rm -f *.lk
	rm -f *.lst
	rm -f *~
	rm -f *.noi
	rm -f *.ihx
	rm -f *.map
	rm -f *.asm
	rm -f *.sym
	rm -f *.pdb
	rm -f *.ilk
	rm -f *.obj
	rm -f *.dat.h

cleanall: clean
	rm -f *.exe
	rm -f *.cmd
	rm -f *.cas
	rm -f *.dsk
	rm -f *.t8c


tags:
	ctags -e *.h *.c



