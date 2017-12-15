CC=gcc
NAME=gkbd

APP_C=src/main.c

LIB_C=src/device.c src/reader.c src/writer.c
LIB_H=src/gkbd.h

LD_FLAGS=-lhidapi-hidraw -lpthread
C_FLAGS=-Dhidapi

all: bin lib

bin: bin/$(NAME)

bin/$(NAME): $(APP_C) $(LIB_C) $(LIB_H)
	@mkdir -p bin
	$(CC) $(APP_C) $(LIB_C) $(C_FLAGS) $(LD_FLAGS) -o $@

bin-linked: $(APP_C) lib
	@mkdir -p bin
	$(CC) $(APP_C) $(C_FLAGS) $(LD_FLAGS) -l$(NAME) -L./lib -o bin/$(NAME)

lib: lib/lib$(NAME).so

lib/lib$(NAME).so: $(LIB_C) $(LIB_H)
	@mkdir -p lib
	$(CC) $(LIB_C) $(C_FLAGS) $(LD_FLAGS) -fPIC -shared -Wl,-soname,lib$(NAME).so -o $@

clean:
	@rm -rf bin lib
