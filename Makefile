.SUFFIXES: .o .cpp

CXX = g++
CXXFLAGS = -std=c++17 -Wall -pedantic -O3

OBJDIR = object

PROGS = decode 

all: $(PROGS)

cleanImg:
	rm -rf ImageTest/*.ppm

clean: cleanImg
	rm -rf $(PROGS) $(OBJDIR)/*.o

encode: object/encode.o
	$(CXX) $(CXXFLAGS) $^ -o $@

decode: object/decode.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJDIR)/%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@