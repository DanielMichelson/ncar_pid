###########################################################################
# Copyright (C) 2019 The Crown (i.e. Her Majesty the Queen in Right of Canada)
#
# This file is an add-on to RAVE.
#
# RAVE is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# RAVE is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with RAVE.  If not, see <http://www.gnu.org/licenses/>.
# ------------------------------------------------------------------------
# 
# ncar_pid/src/Makefile
# @file
# @author Daniel Michelson, Environment and Climate Change Canada
# @date 2019-11-15
###########################################################################
-include $(RAVEROOT)/rave/mkf/def.mk

NCAR_PIDINC= -I. $(NUMPY_INCLUDE_DIR) $(INCLUDE_PYTHON) \
		-I$(HLHDF_INCLUDE_DIR) $(ZLIB_INCDIR) $(HDF5_INCDIR) \
		$(PROJ_INCLUDE_DIR) $(RAVE_INCLUDE_FLAG)

LDFLAGS+= -L. -L$(HLHDF_LIB_DIR) $(ZLIB_LIBDIR) $(HDF5_LIBDIR) $(PROJ_LIB_DIR) \
		$(RAVE_LIB_FLAG)

ifeq ($(EXPAT_SUPPRESSED), no)
NCAR_PIDINC+= $(EXPAT_INCLUDE_DIR)
LDFLAGS+= $(EXPAT_LIB_DIR)
endif

ifeq ($(BUFR_SUPPRESSED), no)
NCAR_PIDINC+= $(BUFR_INCLUDE_DIR)
LDFLAGS+= $(BUFR_LIB_DIR)
endif

CFLAGS=	$(OPTS) $(CCSHARED) $(DEFS) $(CREATE_ITRUNC) $(NCAR_PIDINC)

# --------------------------------------------------------------------
# Fixed definitions

NCAR_PIDSOURCES= BeamHeight.cc FilterUtils.cc NcarParticleId.cc PidImapManager.cc PidInterestMap.cc TaStr.cc TempProfile.cc KdpFilt.cc ncar_pid.cc
INSTALL_HEADERS= BeamHeight.hh FilterUtils.hh NcarParticleId.hh PidImapManager.hh PidInterestMap.hh TaStr.hh TempProfile.hh KdpFilt.hh ncar_pid.h
NCAR_PIDOBJS= $(NCAR_PIDSOURCES:.cc=.o)
LIBNCAR_PID= libncar_pid.so
NCAR_PIDMAIN= 
NCAR_PIDBIN= ncar_pid
NCAR_PIDBINDEB= ncar_pid_deb
NCAR_PIDLIBS= -lncar_pid $(RAVE_MODULE_LIBRARIES)

MAKEDEPEND=gcc -MM $(CFLAGS) -o $(DF).d $<
DEPDIR=.dep
DF=$(DEPDIR)/$(*F)
# --------------------------------------------------------------------
# Rules

# Contains dependency generation as well, so if you are not using
# gcc, comment out everything until the $(CC) statement.
%.o : %.cc
	@$(MAKEDEPEND); \
	cp $(DF).d $(DF).P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $(DF).d >> $(DF).P; \
	\rm -f $(DF).d
	$(CC) -c $(CFLAGS) $<

# Ensures that the .dep directory exists
.PHONY=$(DEPDIR)
$(DEPDIR):
	+@[ -d $@ ] || mkdir -p $@

.PHONY=all
all:		$(LIBNCAR_PID) #bin

$(LIBNCAR_PID): $(DEPDIR) $(NCAR_PIDOBJS) 
	$(LDSHARED) -o $@ $(NCAR_PIDOBJS)

.PHONY=bin
bin: 
	$(CC) $(NCAR_PIDINC) $(LDFLAGS) -o $(NCAR_PIDBIN) $(NCAR_PIDOBJS) $(NCAR_PIDLIBS)

.PHONY=bin-debug
bin-debug:		$(NCAR_PIDBINDEB)

$(NCAR_PIDBINDEB): $(NCAR_PIDSOURCES)
	$(CC) $(CFLAGS) -c ncar_pid.c -o ncar_pid.o
	$(CC) $(CFLAGS) $(NCAR_PIDINC) $(LDFLAGS) -o $(NCAR_PIDBINDEB) $(NCAR_PIDOBJS) $(NCAR_PIDLIBS)

.PHONY=install
install:
	@"$(HLHDF_INSTALL_BIN)" -f -o -C $(LIBNCAR_PID) "$(prefix)/lib/$(LIBNCAR_PID)"
	@for i in $(INSTALL_HEADERS) ; \
	do \
		"$(HLHDF_INSTALL_BIN)" -f -o -m644 -C $$i "$(prefix)/include/$$i"; \
	done
#	"$(HLHDF_INSTALL_BIN)" -f -o -C $(NCAR_PIDBIN) "$(prefix)/bin/$(NCAR_PIDBIN)";

.PHONY=clean
clean:
		@\rm -f *.o core *~ $(LIBNCAR_PID)
#		@\rm -fr $(DEPDIR)

.PHONY=distclean		 
distclean:	clean
		@\rm -f *.so

# NOTE! This ensures that the dependencies are setup at the right time so this should not be moved
-include $(NCAR_PIDSOURCES:%.cc=$(DEPDIR)/%.P)
