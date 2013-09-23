
#-----------------------------------------------------------------------------
# On Linux,    this should be 'Linux'
# On Mac OS X, this should be 'MAC_OS_X'
#-----------------------------------------------------------------------------

PLATFORM := $(shell uname -s)
MACHINE := $(shell uname -m)

ifeq ($(PLATFORM),Linux)
  ifeq ($(MACHINE),x86_64)
    PLATFORM := LINUX64
  else
    PLATFORM := LINUX
  endif
else
  ifeq ($(PLATFORM),Darwin)
    PLATFORM := MAC_OS_X
  else
    PLATFORM := OTHER
  endif
endif


#-----------------------------------------------------------------------------
# Choose a compiler & its options
#--------------------------------------------------------------------------

CXX      = g++
LINK     = g++

OPTS = -ansi -MMD -Wall -O3 -Wno-deprecated  
ifeq ($(PLATFORM),MAC_OS_X)
 OPTS += -DMACOS
endif

#--------------------------------------------------------------------
# image lib
#--------------------------------------------------------------------

ifeq ($(PLATFORM),MAC_OS_X)
   IMG_LIB = -L/opt/local/lib -ljpeg -lpng
else
   IMG_LIB = -ljpeg -lpng
endif

#--------------------------------------------------------------------
# boost
#--------------------------------------------------------------------
BOOST = boost_1_50_0


#--------------------------------------------------------------------
# Put all together
#--------------------------------------------------------------------
TARGET  = hd
SUBDIRS = . src include third_party/rapidxml-1.13 third_party util ${BOOST}
INCLUDE = $(addprefix -I,$(SUBDIRS))
SRCS=$(wildcard $(addsuffix /*.cpp,$(SUBDIRS)))
OBJS=${SRCS:.cpp=.o}
LIB = $(IMG_LIB) 

CFLAGS   = $(OPTS) $(INCLUDE) 
CXXFLAGS = $(CFLAGS) 


.PHONY: all
all :  $(TARGET)

#--------------------------------------------------------------------

#--------------------------------------------------------------------
$(TARGET): $(OBJS)
	${CXX} ${CXXFLAGS} -o $@ $(OBJS) $(LIB)

clean:
	-rm -f $(OBJS) $(TARGET) Dependencies

#--------------------------------------------------------------------
.SUFFIXES: .cpp

.cpp.o:
	${CXX} ${CXXFLAGS} -c $< -o $@
	cat $*.d >> Dependencies
	rm -f $*.d 

Dependencies:
	touch Dependencies

include Dependencies