CC = gcc

INCDIRS = -Iinclude
LIBDIRS = -Llib
LDDIRS = -lhttp-server -lpthread

CFLAGS = -std=c99 -Wall -Wextra -Werror -Wno-unused-parameter $(INCDIRS)
LDFLAGS = $(LIBDIRS) $(LDDIRS) -Wl,-rpath=lib

SRCS = test/logger.c \
	   test/server.c

OBJS = $(patsubst %, bin/%, $(SRCS:.c=.test.out))

TARGET = bin/unit_tester

ifneq ($(BUILD),debug)
CFLAGS += -O2 -s
else
CFLAGS += -Og -g -D_DEBUG
endif

.PHONY: all clean

all: $(TARGET)
	
$(TARGET): build/test/unit_tester.o $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

build/test/%.o: test/%.c
	@mkdir -p $(dir $@)
	$(CC) -o $@ -c $< $(CFLAGS)

bin/test/%.test.out: test/%.c
	@mkdir -p $(dir $@)
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(OBJS)
