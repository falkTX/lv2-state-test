# lv2-state-test

CC ?= gcc

CFLAGS += -O0 -g -Wall -Wextra -std=gnu99
CFLAGS += -fPIC -DPIC
CFLAGS += $(shell pkg-config --cflags lv2)

ifneq ($(shell pkg-config --atleast-version=1.18 lv2 && echo true),true)
CFLAGS += -DDO_NOT_HAVE_LV2_STATE_FREE_PATH
endif

LDFLAGS += -Wl,--no-undefined
LDFLAGS += $(shell pkg-config --libs lv2)

TARGET = lv2-state-test.lv2/lv2-state-test.so

all: $(TARGET)

$(TARGET): lv2-state-test.c
	$(CC) $< $(CFLAGS) $(LDFLAGS) -shared -o $@

clean:
	rm -f $(TARGET)
