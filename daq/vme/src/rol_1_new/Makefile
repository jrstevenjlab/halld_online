#
# File:
#    Makefile
#
# Description:
#    Makefile for the CODA 3.0 primary and secondary readout lists 
#    running on an Intel-based controller running Linux
#
#
# Uncomment DEBUG line for debugging info ( -g and -Wall )
DEBUG=1
#
#HIST=0 

HIST = 1

# Plug in your primary readout lists here..
VMEROL		= ti_master_list.so vme_list.so  ts_list.so dsc_list.so  gt_list.so
# Add shared library dependencies here.  (jvme, ti, td, ts, sd, ctp are already included)
ROLLIBS		= -lfadc

# LINUXVME_LIB	?= ../lib
# LINUXVME_INC	?= ../include
# LINUXVME_BIN	?= ../bin

LINUXVME_LIB	=  /gluondaqfs/hdops/CDAQ/daq_dev_v0.31/daq/vme/src/test/lib
LINUXVME_INC	=  /gluondaqfs/hdops/CDAQ/daq_dev_v0.31/daq/vme/src/test/include
LINUXVME_BIN    ?= ../bin



# DEFs for compiling primary readout lists
CC			= gcc
AR                      = ar
RANLIB                  = ranlib
ifdef DEBUG
CFLAGS			= -Wall -g
else
CFLAGS			= -O3
endif
CFLAGS			+= -w -I. -I${LINUXVME_INC} -I/usr/include -I${CODA}/common/include\
			  -L. -L${LINUXVME_LIB} -DJLAB
CFLAGS			+= -lrt -rdynamic -lpthread -ljvme -lti -ltd -lts -lsd -lctp -lssp -lgtp -lfa125 -ldsc2 -lcaen1190 -lf1tdc -lsrs -lhd -llz4 $(ROLLIBS)
CFLAGS			+= -DLINUX -DDAYTIME=\""`date`"\"

#====================  ROOTSPY =============================
ifeq ($(HIST),1)
	CFLAGS	+= -DSHM_BUF  -I ../rcm/monitor -x c ../rcm/monitor/hbook.cc -x none
endif
#===========================================================

# DEFs for compiling CODA readout lists
CCRL			= ${CODA_BIN}/ccrl
CODA_INCS		= -I. -I${LINUXVME_INC} -I${CODA}/common/include
CODA_LIBDIRS            = -L. -L${LINUXVME_LIB}
CODA_LIBS		= -rdynamic -ljvme -lti -ltd -lts -lsd -lctp -lssp -lgtp -lfa125 -ldsc2 -lcaen1190 -lf1tdc
CODA_DEFS		= -DLINUX -DDAYTIME=\""`date`"\" -DINIT_NAME=$(@:.so=__init) -DINIT_NAME_POLL=$(@:.so=__poll)
CODA_CFLAGS		= -O -w -fpic -shared ${CODA_INCS} ${CODA_LIBDIRS} \
			  ${CODA_LIBS} ${CODA_DEFS}
ifdef DEBUG
CODA_CFLAGS		+= -Wall -g
endif
CRLFILES		= $(wildcard *.crl)
CFILES			= $(CRLFILES:.crl=.c)
SOBJS			= $(CRLFILES:.crl=.so)

ifeq ($(ARCH),x86_64)
	libdir:=/usr/lib64/gef
else
	libdir:=/usr/lib/gef
endif

# End in error, if the gefvme-kmod-common RPM is not installed.
ifeq (,$(wildcard $(libdir)/libvme.a))
$(error gefvme-kmod-common RPM not installed!)
endif

all: $(VMEROL) $(SOBJS) 

event_list.c: event_list.crl Makefile
	@echo
	@echo "Converting $< to $@"
	@${CCRL} $<

event_list.so: event_list.c
	@echo
	@echo "Building $@ from $<"
	@echo
	${CC} ${CODA_CFLAGS} -o $@ $<

%.c: %.crl Makefile
	@echo
	@echo "Converting $< to $@"
	@${CCRL} $<

%.so: %.c Makefile
	@echo
	@echo "Building $@ from $<"
	@echo
	$(CC) -fpic -shared  $(CFLAGS) -DINIT_NAME=$(@:.so=__init) -DINIT_NAME_POLL=$(@:.so=__poll) -o $@ $<

clean distclean:
	@rm -f  $(VMEROL) $(SOBJS) $(CFILES) *~ 

.PHONY: all
