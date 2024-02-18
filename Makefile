CC := gcc
CFLAGS := -Iincludes -Wall -Wextra -Werror -Wpedantic
LDFLAGS := -lpthread
SRC_DIR := src
BIN_DIR := bin
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(SRC_FILES))
TARGET := $(BIN_DIR)/server

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean
