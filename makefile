CXX = g++ -std=c++11 -Wall
INCLUDE = -I ./ising
OUTPUT = -o ising-run

SRC = \
	./ising/ising-run/main.cpp             \
	./ising/ising-core/fast-rand.cpp       \
	./ising/ising-core/timing.cpp          \
	./ising/ising-core/ising-parameter.cpp \
	./ising/ising-core/ising-2d.cpp

all:
	$(CXX) $(INCLUDE) $(SRC) $(OUTPUT)

clean:
	rm -f *.o
