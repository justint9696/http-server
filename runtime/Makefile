CC = gcc

INCDIRS = -Iinclude
LIBDIRS = -Llib
LDLIBS = -lhttp-server

CFLAGS = -std=c99 -Wall -Wextra -Werror -Wno-unused-parameter $(INCDIRS)
LDFLAGS = $(LIBDIRS) $(LDLIBS) -Wl,-rpath=lib

SRCS = runtime/main.c
OBJS = $(patsubst %, build/%, $(SRCS:.c=.o))
DEPS = $(OBJS:.o=.d)

TARGET = bin/server

ifneq ($(BUILD),debug)
CFLAGS += -O2 -s
else
CFLAGS += -Og -g -D_DEBUG
endif

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(LDFLAGS)

build/runtime/%.o: runtime/%.c
	@mkdir -p $(dir $@)
	$(CC) -o $@ -c $< $(CFLAGS) -MMD

clean:
	rm -f $(TARGET) $(DEPS) $(OBJS)

-include $(DEPS)
