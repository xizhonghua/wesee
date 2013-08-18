#
# makefile 
#
# LIB_DIR
# SRC_DIR
#

TARGET = matting
SRC_DIR=./src
LIB_DIR=./libs

#
# Determine the platform (Linux, Mac OS X, Windows+Cygwin etc.)
#
# On Linux,    this should be 'Linux'
# On Mac OS X, this should be 'Darwin'
#

OSNAME := $(shell uname -s)
MACHINE := $(shell uname -m)

ifeq ($(OSNAME),Linux)
  ifeq ($(MACHINE),x86_64)
    PLATFORM := LINUX64
  else
    PLATFORM := LINUX
  endif
else 
  ifeq ($(OSNAME),Darwin)
    PLATFORM := MAC_OS_X
  else
    PLATFORM := OTHER
  endif
endif

#--------------------------------------------------------------------------
# Choose a compiler & its options
#--------------------------------------------------------------------------
CXX = g++

ifeq ($(PLATFORM),MAC_OS_X)
  OPTS = -pipe -O3 -MMD  -DMACOS -Wreturn-type -std=gnu++0x
else 
  OPTS = -pipe -O3 -MMD -std=gnu++0x
endif


#--------------------------------------------------------------------
# Code structure 
#--------------------------------------------------------------------

MT_SUB_DIRS = $(SRC_DIR)

#MP_SUB_DIRS = $(SRC_DIR)/mathtool \
              $(SRC_DIR)/mtrand \
              $(SRC_DIR)/graph \
              $(SRC_DIR)/loader \
              $(SRC_DIR)/collisionprediction \
			  $(SRC_DIR)/old-collisionpredict \
              $(SRC_DIR)/OBB \
              $(SRC_DIR)/acd2d $(SRC_DIR)/acd2d/edge_visibility $(SRC_DIR)/acd2d/hull \
              $(SRC_DIR)/Eigen-src \
              $(SRC_DIR) \
              $(SRC_DIR)/nlopt \
              tmmp \
              sampler/GL \
              sampler \
              

MT_INCL=$(addprefix -I,$(MT_SUB_DIRS))

#--------------------------------------------------------------------
INCLUDE = $(MT_INCL) -I.
LIB = -lm

#--------------------------------------------------------------------

CFLAGS     = $(OPTS) $(INCLUDE)
CXXFLAGS   = $(CFLAGS)

#--------------------------------------------------------------------
SRCS=$(wildcard $(addsuffix /*.cpp,$(MT_SUB_DIRS)))
OBJS=${SRCS:.cpp=.o}
DFILES=${SRCS:.cpp=.d}

####### Build rules

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) ${CXXFLAGS} -o $@ $(OBJS) $(LIB) 

clean:
	-rm -f $(OBJS) $(TARGET) $(DFILES) Dependencies

#--------------------------------------------------------------------
.SUFFIXES: .cpp

.cpp.o: 
	${CXX} ${CXXFLAGS} -c $< -o $@
	cat $*.d >> Dependencies
	rm -f $*.d

Dependencies:
	touch Dependencies

-include Dependencies
