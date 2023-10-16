#! make

SOURCE = $(wildcard *.cpp)
TARGETS = $(SOURCE:.cpp=)
HEADERS = Number.h LychrelData.h

STD = -std=c++20
OPTIMIZER = -O3
CXXFLAGS = $(STD) $(OPTIMIZER) $(HEADERS)

default: $(TARGETS)

clean clobber:
	$(RM) $(TARGETS)
