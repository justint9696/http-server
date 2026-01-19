CC = gcc
LD = gcc

INCDIRS = -Ihttp-server
LIBDIRS =
LDDIRS =

CFLAGS = -std=c99 -Wall -Wextra -Werror -Wno-unused-parameter $(INCDIRS)
LDFLAGS = $(LIBDIRS) $(LDDIRS)

SRCS = http-server/logger.c \
       http-server/server.c
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
