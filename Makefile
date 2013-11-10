CC ?= gcc
SRC_DIR ?= src
BUILD_DIR ?= build
SRCS = \
	$(SRC_DIR)/dns.c \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/sandbox.c \
	$(SRC_DIR)/register.c \
	$(SRC_DIR)/scheduling.c \
	$(SRC_DIR)/util.c
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

EXE ?= censorscope
CFLAGS += `pkg-config lua5.1 --cflags` -g -Wall -std=gnu99
LDFLAGS += `pkg-config lua5.1 --libs` -lldns

all: $(EXE)
	echo $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(EXE): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -f $(OBJS)

clobber: clean
	rm $(EXE)
