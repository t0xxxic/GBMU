CC=clang
CFLAGS=-I./include -g

BUILD_DIR = obj
TARGET = gbmu

C_FILES = $(wildcard src/*.c)
OBJ = $(C_FILES:src/%.c=$(BUILD_DIR)/%.o)
DEP = $(OBJ:%.o=%.d)

$(TARGET) : $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@  

-include $(DEP)

$(BUILD_DIR)/%.o : src/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJ) $(DEP)