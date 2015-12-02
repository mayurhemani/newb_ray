OUTPUT= newb_ray
BINS_DIR= bin
INCLUDES= 	-Iext/glm/ \
			-I/usr/local/include \
			-Iinclude \
			-Iext/rapidjson/include/

LIBS_DIR= -L/usr/local/lib

LIBS= -lopencv_core -lopencv_highgui -lopencv_imgproc

CCFLAGS= -std=c++11 -O0 -g
	
all: $(BINS_DIR)/newbray.o $(BINS_DIR)/donkey.o $(BINS_DIR)/main.o
	g++ $(BINS_DIR)/main.o $(BINS_DIR)/donkey.o $(BINS_DIR)/newbray.o -o $(OUTPUT) $(LIBS_DIR) $(LIBS)

$(BINS_DIR)/newbray.o: include/donkey.h include/newbray.h src/newbray.cpp
	g++ -c src/newbray.cpp $(INCLUDES) -o $(BINS_DIR)/newbray.o $(CCFLAGS)

$(BINS_DIR)/donkey.o: include/donkey.h src/donkey.cpp
	g++ -c src/donkey.cpp $(INCLUDES) -o $(BINS_DIR)/donkey.o $(CCFLAGS)

$(BINS_DIR)/main.o: include/donkey.h include/newbray.h src/main.cpp
	g++ -c src/main.cpp $(INCLUDES) -o $(BINS_DIR)/main.o $(CCFLAGS)
