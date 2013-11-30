CC ?= gcc

SRC_DIR ?= src
EXT_DIR ?= ext
BUILD_DIR ?= build
SRCS = \
	$(SRC_DIR)/censorscope.c \
	$(SRC_DIR)/dns.c \
	$(SRC_DIR)/experiment.c \
	$(SRC_DIR)/http.c \
	$(SRC_DIR)/logging.c \
	$(SRC_DIR)/options.c \
	$(SRC_DIR)/register.c \
	$(SRC_DIR)/sandbox.c \
	$(SRC_DIR)/scheduling.c \
	$(SRC_DIR)/subprocesses.c \
	$(SRC_DIR)/tcp.c \
	$(SRC_DIR)/termination.c \
	$(SRC_DIR)/transport.c \
	$(SRC_DIR)/util.c \
	$(EXT_DIR)/ini.c
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

TEST_DIR ?= tests
TEST_SRCS = \
	$(SRC_DIR)/util.c \
	$(EXT_DIR)/tinytest.c \
	$(TEST_DIR)/tests.c
TEST_OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(TEST_SRCS))

EXE ?= censorscope
TEST_EXE ?= censorscope-tests
LUA_CFLAGS ?= `pkg-config lua5.1 --cflags`
CFLAGS += $(LUA_CFLAGS) -g -Wall -Werror -std=gnu99
ifdef DEFAULT_SANDBOX_DIR
CFLAGS += -DDEFAULT_SANDBOX_DIR="\"$(DEFAULT_SANDBOX_DIR)\""
endif
ifdef DEFAULT_LUASRC_DIR
CFLAGS += -DDEFAULT_LUASRC_DIR="\"$(DEFAULT_LUASRC_DIR)\""
endif
ifdef DEFAULT_RESULTS_DIR
CFLAGS += -DDEFAULT_RESULTS_DIR="\"$(DEFAULT_RESULTS_DIR)\""
endif
ifdef DEFAULT_MAX_MEMORY
CFLAGS += -DDEFAULT_MAX_MEMORY="$(DEFAULT_MAX_MEMORY)"
endif
ifdef DEFAULT_MAX_INSTRUCTIONS
CFLAGS += -DDEFAULT_MAX_INSTRUCTIONS="$(DEFAULT_MAX_INSTRUCTIONS)"
endif
ifdef DEFAULT_DOWNLOAD_TRANSPORT
CFLAGS += -DDEFAULT_DOWNLOAD_TRANSPORT="\"$(DEFAULT_DOWNLOAD_TRANSPORT)\""
endif
ifdef DEFAULT_UPLOAD_TRANSPORT
CFLAGS += -DDEFAULT_UPLOAD_TRANSPORT="\"$(DEFAULT_UPLOAD_TRANSPORT)\""
endif
LUA_LIBS ?= `pkg-config lua5.1 --libs`
LDFLAGS := $(LUA_LIBS) -lldns -levent -lcurl -lssl -lcrypto -lz $(LDFLAGS)

all: $(EXE)
	echo $(BUILD_DIR)

$(BUILD_DIR)/src/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(EXT_DIR)/%.o: $(EXT_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(EXE): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

$(TEST_EXE): $(TEST_OBJS)
	$(CC) $(LDFLAGS) $(TEST_OBJS) -o $@

test: $(TEST_EXE)
	./$(TEST_EXE)

clean:
	rm -f $(OBJS) $(TEST_OBJS)
	rm -rf $(BUILD_DIR)

clobber: clean
	rm -f $(EXE) $(TEST_EXE)
