CC ?= gcc
SRC_DIR ?= src
BUILD_DIR ?= build
SRCS = \
	$(SRC_DIR)/censorscope.c \
	$(SRC_DIR)/dns.c \
	$(SRC_DIR)/http.c \
	$(SRC_DIR)/options.c \
	$(SRC_DIR)/register.c \
	$(SRC_DIR)/sandbox.c \
	$(SRC_DIR)/scheduling.c \
	$(SRC_DIR)/tcp.c \
	$(SRC_DIR)/transport.c \
	$(SRC_DIR)/util.c
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

EXE ?= censorscope
LUA_CFLAGS ?= `pkg-config lua5.1 --cflags`
CFLAGS += $(LUA_CFLAGS) -g -Wall -std=gnu99
ifdef DEFAULT_SANDBOX_DIR
CFLAGS += -DDEFAULT_SANDBOX_DIR="\"$(DEFAULT_SANDBOX_DIR)\""
endif
ifdef DEFAULT_LUASRC_DIR
CFLAGS += -DDEFAULT_LUASRC_DIR="\"$(DEFAULT_LUASRC_DIR)\""
endif
ifdef DEFAULT_MAX_MEMORY
CFLAGS += -DDEFAULT_MAX_MEMORY="$(DEFAULT_MAX_MEMORY)"
endif
ifdef DEFAULT_MAX_INSTRUCTIONS
CFLAGS += -DDEFAULT_MAX_INSTRUCTIONS="$(DEFAULT_MAX_INSTRUCTIONS)"
endif
LUA_LIBS ?= `pkg-config lua5.1 --libs`
LDFLAGS += $(LUA_LIBS) -lldns -levent -lcurl -lssl -lcrypto -lz

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
