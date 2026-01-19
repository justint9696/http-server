CC = gcc

INCDIRS = -Ihttp-server
LIBDIRS = -Llib
LDDIRS = -lhttp-server

CFLAGS = -std=c99 -Wall -Wextra -Werror -Wno-unused-parameter $(INCDIRS)
LDFLAGS = $(LIBDIRS) $(LDDIRS) -Wl,-rpath=lib

SRCS = src/main.c
OBJS = $(patsubst %, build/%, $(SRCS:.c=.o))
DEPS = $(OBJS:.o=.d)

TARGET = bin/server

ifneq ($(BUILD),debug)
CFLAGS += -O2 -s
else
CFLAGS += -Og -g -D_DEBUG
endif

.PHONY: all release

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(LDFLAGS)

build/src/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) -o $@ -c $< $(CFLAGS) -MMD

clean:
	rm -f $(TARGET) $(DEPS) $(OBJS)

-include $(DEPS)
