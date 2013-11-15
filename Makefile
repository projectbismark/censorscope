CC ?= gcc
SRC_DIR ?= src
BUILD_DIR ?= build
SRCS = \
	$(SRC_DIR)/dns.c \
	$(SRC_DIR)/tcp.c \
	$(SRC_DIR)/http.c \
	$(SRC_DIR)/censorscope.c \
	$(SRC_DIR)/sandbox.c \
	$(SRC_DIR)/register.c \
	$(SRC_DIR)/scheduling.c \
	$(SRC_DIR)/transport.c \
	$(SRC_DIR)/util.c
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

EXE ?= censorscope
CFLAGS += `pkg-config lua5.1 --cflags` -g -Wall -std=gnu99
LDFLAGS += `pkg-config lua5.1 --libs` -lldns -levent -lcurl -lssl -lcrypto -lz

all: $(EXE)
	echo $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(EXE): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -f $(OBJS)
	rm -rf $(BUILD_DIR)

clobber: clean
	rm $(EXE)
