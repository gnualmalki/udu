#
# Check if compiler sup LTO
#
LTO_OK := $(shell echo 'int main(){return 0;}' | $(CC) -x c - -flto -o /dev/null 2>/dev/null && echo "OK")

ifeq ($(LTO_OK),OK)
    CFLAGS  += -flto
    LDFLAGS += -flto
    LTO_MSG := enabled
else
    LTO_MSG := not supported
endif
