CC=gcc
SRC=$(wildcard ./src/*.c)
HEADERS=$(wildcard ./src/*.h)
CFLAGS=-O3 -Wall -Wextra
DFLAGS=-O0 -g -Wall -Wextra
LFLAGS=
BUILD=build
EXE=$(BUILD)/typing-test
DEBUG=$(BUILD)/typing-test-deb

.PHONY: clean run debug

all: $(EXE) $(DEBUG)

$(EXE): $(BUILD) $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $(EXE) $(SRC) $(LFLAGS)

$(DEBUG): $(BUILD) $(SRC) $(HEADERS)
	$(CC) $(DFLAGS) -o $(DEBUG) $(SRC) $(LFLAGS)

run: $(EXE)
	./$(EXE)

debug: $(DEBUG)
	gdb ./$(DEBUG)

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
