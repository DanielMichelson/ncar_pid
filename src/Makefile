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
# ncarb/src/Makefile
# @file
# @author Daniel Michelson, Environment and Climate Change Canada
# @date 2019-11-15
###########################################################################
-include $(RAVEROOT)/rave/mkf/def.mk

NCARBINC= -I. $(NUMPY_INCLUDE_DIR) $(INCLUDE_PYTHON) \
		-I$(HLHDF_INCLUDE_DIR) $(ZLIB_INCDIR) $(HDF5_INCDIR) \
		$(PROJ_INCLUDE_DIR) $(RAVE_INCLUDE_FLAG)

LDFLAGS+= -L. -L$(HLHDF_LIB_DIR) $(ZLIB_LIBDIR) $(HDF5_LIBDIR) $(PROJ_LIB_DIR) \
		$(RAVE_LIB_FLAG)

ifeq ($(EXPAT_SUPPRESSED), no)
NCARBINC+= $(EXPAT_INCLUDE_DIR)
LDFLAGS+= $(EXPAT_LIB_DIR)
endif

ifeq ($(BUFR_SUPPRESSED), no)
NCARBINC+= $(BUFR_INCLUDE_DIR)
LDFLAGS+= $(BUFR_LIB_DIR)
endif

CFLAGS=	$(OPTS) $(CCSHARED) $(DEFS) $(CREATE_ITRUNC) $(NCARBINC)

# --------------------------------------------------------------------
# Fixed definitions

NCARBSOURCES= BeamHeight.cc FilterUtils.cc NcarParticleId.cc PidImapManager.cc PidInterestMap.cc TaStr.cc TempProfile.cc KdpFilt.cc ncar_pid.cc
INSTALL_HEADERS= BeamHeight.hh FilterUtils.hh NcarParticleId.hh PidImapManager.hh PidInterestMap.hh TaStr.hh TempProfile.hh KdpFilt.hh ncar_pid.h
NCARBOBJS= $(NCARBSOURCES:.cc=.o)
LIBNCARB= libncarb.so
NCARBMAIN= 
NCARBBIN= ncarb
NCARBBINDEB= ncarb_deb
NCARBLIBS= -lncarb $(RAVE_MODULE_LIBRARIES)

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
all:		$(LIBNCARB) #bin

$(LIBNCARB): $(DEPDIR) $(NCARBOBJS) 
	$(LDSHARED) -o $@ $(NCARBOBJS)

.PHONY=bin
bin: 
	$(CC) $(NCARBINC) $(LDFLAGS) -o $(NCARBBIN) $(NCARBOBJS) $(NCARBLIBS)

.PHONY=bin-debug
bin-debug:		$(NCARBBINDEB)

$(NCARBBINDEB): $(NCARBSOURCES)
	$(CC) $(CFLAGS) -c ncarb.c -o ncarb.o
	$(CC) $(CFLAGS) $(NCARBINC) $(LDFLAGS) -o $(NCARBBINDEB) $(NCARBOBJS) $(NCARBLIBS)

.PHONY=install
install:
	@"$(HLHDF_INSTALL_BIN)" -f -o -C $(LIBNCARB) "$(prefix)/lib/$(LIBNCARB)"
	@for i in $(INSTALL_HEADERS) ; \
	do \
		"$(HLHDF_INSTALL_BIN)" -f -o -m644 -C $$i "$(prefix)/include/$$i"; \
	done
#	"$(HLHDF_INSTALL_BIN)" -f -o -C $(NCARBBIN) "$(prefix)/bin/$(NCARBBIN)";

.PHONY=clean
clean:
		@\rm -f *.o core *~ $(LIBNCARB)
#		@\rm -fr $(DEPDIR)

.PHONY=distclean		 
distclean:	clean
		@\rm -f *.so

# NOTE! This ensures that the dependencies are setup at the right time so this should not be moved
-include $(NCARBSOURCES:%.cc=$(DEPDIR)/%.P)
