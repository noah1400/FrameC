CC := gcc
CFLAGS := -Iincludes -Wall -Wextra -Werror -Wpedantic -g
LDFLAGS := -lpthread
SRC_DIR := src
BIN_DIR := bin
OBJS_DIR := $(BIN_DIR)/objs
VIEWS_DIR := views
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJS_DIR)/%.o,$(SRC_FILES))
TARGET := $(BIN_DIR)/server

all: $(TARGET) copy_views

$(TARGET): $(OBJ_FILES)
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJS_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJS_DIR):
	mkdir -p $@

copy_views: | $(BIN_DIR)
	cp -r $(VIEWS_DIR) $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean copy_views
