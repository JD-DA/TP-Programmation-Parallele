CXX=mpicxx
CC=$(CXX)
SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o)
EXEC=main

all: $(EXEC)

$(EXEC): $(OBJ) 

.PHONY: clean mrproper

clean:
	@rm -rf $(OBJ)

mrproper: clean
	@rm -rf $(EXEC)
