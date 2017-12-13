CC=gcc
NAME=g610

APP_C=src/main.c

LIB_C=src/device.c src/key.c src/reader.c src/writer.c
LIB_H=src/g610.h

LD_FLAGS=-lhidapi-hidraw
C_FLAGS=-Dhidapi

all: bin lib

bin: bin/$(NAME)

bin/$(NAME): $(APP_C) lib
	@mkdir -p bin
	$(CC) $(APP_C) $(C_FLAGS) $(LD_FLAGS) -l$(NAME) -Llib -o $@

lib: lib/lib$(NAME).so

lib/lib$(NAME).so: $(LIB_C) $(LIB_H)
	@mkdir -p lib
	$(CC) $(LIB_C) $(C_FLAGS) $(LD_FLAGS) -fPIC -shared -Wl,-soname,$@ -o $@

clean:
	@rm -rf bin lib
