BIN=snake
LDFLAGS=$(shell sdl2-config --libs) -lGL
CFLAGS=$(shell sdl2-config --cflags) -std=c99 -pedantic
RM=rm -f

all: $(BIN)

$(BIN):

clean:
	$(RM) $(BIN)
