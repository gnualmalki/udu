#
# Check if compiler sup LTO
#
LTO_OK := $(shell echo 'int main(){return 0;}' | \
           $(CC) -x c - -flto -o /dev/null 2>/dev/null && echo "OK")

ifeq ($(LTO_OK),OK)
    $(info [INFO]: LTO enabled)
    CFLAGS  += -flto
    LDFLAGS += -flto
else
    $(warning [WARNING]: LTO not supported)
endif
