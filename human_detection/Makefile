CC=g++
OBJ = src/classifier.o src/config.o src/hog_features.o src/hypothesis.o src/image2hits.o src/model.o src/object_hypothesis.o src/poselet.o src/poselet_api.o src/poselet_cluster.o src/poselet_detector.o src/poselet_hit.o src/mat_imresize.o include/gil_draw.o main.o

BOOST = ../../../boost_1_50_0

CFLAGS  = -O3 -Wall -fPIC -msse4.2 #-fopenmp
#CFLAGS  = -g -Wall -fPIC -msse4.2 #-fopenmp
INCLUDES = -Isrc -Iinclude -Ithird_party/rapidxml-1.13 -I$(BOOST) -Ithird_party -Iutil
TESTLDFLAGS = -Llib -ljpeg -lpng
OUT = lib/libposelet.a

SRC=$(OBJ:.o=.cpp)

$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
	
.PHONY: all
all  : $(OUT)
	$(CC) $(CFLAGS) $(INCLUDES) $(TESTLDFLAGS) main.cpp include/gil_imdebug.cpp -lposelet -ljpeg -lpng -o human_detect

clean:
	rm -f $(OBJ) $(OUT)
