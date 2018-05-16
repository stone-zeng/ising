CXX = g++ -std=c++11 -Wall
INCLUDE = -I ./ising
OUTPUT = -o ./bin/ising

SRC = \
	./ising/core/exact.cpp        \
	./ising/core/fast-rand.cpp    \
	./ising/core/info.cpp         \
	./ising/core/ising-2d.cpp     \
	./ising/core/lattice-data.cpp \
	./ising/core/parameter.cpp    \
	./ising/core/simulation.cpp   \
	./ising/core/timing.cpp       \
	./ising/run/main.cpp

all:
	$(CXX) $(INCLUDE) $(SRC) $(OUTPUT)

clean:
	rm -f *.o
