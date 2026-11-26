CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -Iinclude -Wno-address-of-packed-member
LDFLAGS := -lm

# İşletim sistemini otomatik algıla (macOS vs Linux)
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CC       := clang
    SDK_PATH := $(shell xcrun --sdk macosx --show-sdk-path)
    CFLAGS   += -isysroot $(SDK_PATH)
    LDFLAGS  += -isysroot $(SDK_PATH)
endif

SRC_DIR := src
OBJ_DIR := build
TARGET  := mission_app

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
