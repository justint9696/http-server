CC = gcc
LD = gcc

INCDIRS = -Iinclude
LIBDIRS =
LDLIBS =

CFLAGS = -std=c11 -Wall -Wextra -Werror -Wno-unused-parameter $(INCDIRS)
LDFLAGS = $(LIBDIRS) $(LDLIBS)

SRCS = http-server/logger.c \
       http-server/server.c \
       http-server/client.c \
	   http-server/http.c

OBJS = $(patsubst %, build/%, $(SRCS:.c=.o))
DEPS = $(OBJS:.o=.d)

LOGS = logs

TARGET = lib/libhttp-server.so

ifneq ($(BUILD),debug)
CFLAGS += -O2 -s
else
CFLAGS += -Og -g -D_DEBUG
endif

.PHONY: all clean

$(LOGS):
	@mkdir -p $@

all: $(TARGET) $(LOGS)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(LD) -o $@ $^ $(LDFLAGS) -shared

build/http-server/%.o: http-server/%.c
	@mkdir -p $(dir $@)
	$(CC) -o $@ -c $< $(CFLAGS) -MMD -fPIC

clean:
	rm -f $(TARGET) $(DEPS) $(OBJS)

-include $(DEPS)
