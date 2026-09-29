CC=gcc
SRC=$(wildcard ./src/*.c)
HEADERS=$(wildcard ./src/*.h)
CFLAGS=-O3 -Wall -Wextra
DFLAGS=-O0 -g -Wall -Wextra
LFLAGS=
BUILD=build
EXE=$(BUILD)/typing-test
DEBUG=$(BUILD)/typing-test-deb

all: $(BUILD) $(EXE) $(DEBUG)

$(EXE): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $(EXE) $(SRC) $(LFLAGS)

$(DEBUG): $(SRC) $(HEADERS)
	$(CC) $(DFLAGS) -o $(DEBUG) $(SRC) $(LFLAGS)

run: $(EXE)
	./$(EXE)

debug: $(DEBUG)
	./$(DEBUG)

$(BUILD):
	mkdir -p $(BUILD)

.PHONY: clean
clean:
	rm -rf $(BUILD)
