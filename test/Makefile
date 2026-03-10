CC = gcc

INCDIRS = -Iinclude
LIBDIRS = -Llib
LDDIRS = -lhttp-server -lpthread

CFLAGS = -std=c11 -Wall -Wextra -Werror -Wno-unused-parameter $(INCDIRS)
LDFLAGS = $(LIBDIRS) $(LDDIRS) -Wl,-rpath=lib

SRCS = test/test_logger.c \
       test/test_server.c \
       test/test_http.c \
       test/test_file_io.c

OBJS = $(patsubst %.c, build/%.o, test/unit_tester.c)
DEPS = $(OBJS:.o=.d)
TEST_OBJS = $(patsubst %, bin/%, $(SRCS:.c=.out))

TARGET = bin/unit_tester

ifneq ($(BUILD),debug)
CFLAGS += -O2 -s
else
CFLAGS += -Og -g -D_DEBUG
endif

.PHONY: all clean

all: $(TARGET) $(TEST_OBJS)
	
$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

build/test/%.o: test/%.c
	@mkdir -p $(dir $@)
	$(CC) -o $@ -c $< $(CFLAGS) -MMD

bin/test/%.out: test/%.c
	@mkdir -p $(dir $@)
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(OBJS) $(DEPS) $(TEST_OBJS) $(TARGET)

-include $(DEPS)
