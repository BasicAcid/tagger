CC := gcc
CFLAGS := -O2 \
          -Wall \
          -Wextra \
          -Wformat=2 \
          -Wformat-overflow \
          -Wformat-truncation \
          -Wshadow \
          -Wdouble-promotion \
          -Wundef \
          -fno-common \
          -z noexecstack \
          -Wconversion

SRC := main.c
TARGET := bin/tagger
INSTALL_DIR := ~/.local/bin

.PHONY: build deploy

build: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

deploy: $(INSTALL_DIR)/tagger

$(INSTALL_DIR)/tagger: $(SRC)
	$(CC) $(CFLAGS) -o $@ $^
