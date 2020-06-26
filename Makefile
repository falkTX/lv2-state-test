# lv2-state-test

CC ?= gcc

CFLAGS += -O0 -g -Wall -Wextra -std=gnu99
CFLAGS += $(shell pkg-config --cflags lv2)

LDFLAGS += -Wl,--no-undefined
LDFLAGS += $(shell pkg-config --libs lv2)

TARGET = lv2-state-test.lv2/lv2-state-test.so

all: $(TARGET)

$(TARGET): lv2-state-test.c
	$(CC) $< $(CFLAGS) $(LDFLAGS) -shared -o $@

clean:
	rm -f $(TARGET)
