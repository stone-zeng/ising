CXX = g++ -std=c++11 -Wall
INCLUDE = -I ./ising
OUTPUT = -o ising-run

SRC = \
	./ising/ising-core/exact.cpp      \
	./ising/ising-core/fast-rand.cpp  \
	./ising/ising-core/info.cpp       \
	./ising/ising-core/ising-2d.cpp   \
	./ising/ising-core/parameter.cpp  \
	./ising/ising-core/simulation.cpp \
	./ising/ising-core/timing.cpp     \
	./ising/ising-run/main.cpp

all:
	$(CXX) $(INCLUDE) $(SRC) $(OUTPUT)

clean:
	rm -f *.o
