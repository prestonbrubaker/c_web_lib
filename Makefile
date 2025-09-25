# Compiler and tools
CC = gcc
AR = ar
RM = rm -f

# Compiler flags
CFLAGS = -Wall -O2

# Library and executable names
LIB = libhello.a
EXE = test_hello

# Source and object files
SRC = hello.c
OBJ = $(SRC:.c=.o)
TEST_SRC = test_hello.c

# Default target
all: $(EXE)

# Link test_hello with libhello.a
$(EXE): $(TEST_SRC) $(LIB) hello.h
	$(CC) $(CFLAGS) $(TEST_SRC) -L. -lhello -o $(EXE)

# Create static library
$(LIB): $(OBJ)
	$(AR) rcs $(LIB) $(OBJ)

# Compile source to object file
$(OBJ): $(SRC) hello.h
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

# Clean up generated files
.PHONY: clean
clean:
	$(RM) $(OBJ) $(LIB) $(EXE)
