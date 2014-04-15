#
# makefile 
#
# LIB_DIR
# SRC_DIR
#

TARGET = matting
SRC_DIR=./src

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
  CXX = clang++
  OPTS = -pipe -O3 -MMD -std=c++11 -DMACOS -Wreturn-type -Wwrite-strings
else 
  OPTS = -pipe -O3 -MMD -std=c++11 -Wwrite-strings
endif

#--------------------------------------------------------------------
# OpenCV
#--------------------------------------------------------------------
CV_LIB_BASE = -lopencv_core -lopencv_highgui -lopencv_imgproc
ifeq ($(PLATFORM),MAC_OS_X)
    CV_INCL = -I/opt/local/include
    CV_LIB = -L/opt/local/lib $(CV_LIB_BASE)
else
	CV_LIB = $(CV_LIB_BASE)
endif



#--------------------------------------------------------------------
# Code structure 
#--------------------------------------------------------------------

MT_SUB_DIRS = $(SRC_DIR) \
			$(SRC_DIR)/utils \

MT_INCL=$(addprefix -I,$(MT_SUB_DIRS))

#--------------------------------------------------------------------
INCLUDE = -I. $(MT_INCL) $(CV_INCL)
LIB = -lm $(CV_LIB)

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
