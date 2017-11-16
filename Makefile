#for release
#CXXFLAGS =	-O3 -std=c++11 -Wall -fmessage-length=0
#for profiling
#CXXFLAGS =	-O1 -std=c++11 -g -pg -Wall -fmessage-length=0
#for debugging
CXXFLAGS =	-O0 -g -pg -Wall -Wall -fmessage-length=0 -std=c++11
#for developement
#CXXFLAGS =	-O2 -std=c++11 -fmessage-length=0

CLFLAGS = -DCL_USE_DEPRECATED_OPENCL_2_0_APIS -D__CL_ENABLE_EXCEPTIONS -DCL_ENABLE_EXCEPTIONS

INCLUDE =	-I/usr/local/cuda-9.0/targets/x86_64-linux/include -I${HOME}/projekte/openGLvisualizer 

OBJS =	minimalDemCL.o clhelpers.o initialisation.o evaluation.o

LIBS =	-lOpenCL -L${AMDAPPSDKROOT}/lib/x86_64 -L${HOME}/projekte/openGLvisualizer -lminglvisualizer -lsfml-system -lsfml-window -lGLEW -lSOIL -lGL -lsfml-graphics -lX11 -pthread

TARGET = minimalDemCL

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
	
%.o:	%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(CLFLAGS) -c $*.cpp -o $*.o
	$(CXX) -MM $(CXXFLAGS) $(INCLUDE) $(CLFLAGS) $*.cpp > $*.d

-include $(OBJS:.o=.d)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
