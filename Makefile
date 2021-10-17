# tool macros
AS = arm-none-eabi-as
ASFLAGS := -g
CC := arm-none-eabi-gcc
CCFLAGS := -mthumb -mcpu=cortex-m0plus -nostdlib -ggdb
CCOBJFLAGS := $(CCFLAGS) -c
LD = arm-none-eabi-ld
LDFLAGS = -T
UF := elf2uf2

# path macros
BIN_PATH := bin
OBJ_PATH := obj
SRC_PATH := src


# compile macros
LINK_FILE_NAME := linker.ld
LINK_FILE := $(SRC_PATH)/$(LINK_FILE_NAME)
TARGET_NAME := app
TARGET := $(BIN_PATH)/$(TARGET_NAME)


# src files & obj files
SRC := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.c .s)))
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))


# clean files list
DISTCLEAN_LIST := $(OBJ) \

CLEAN_LIST := $(TARGET) \
			  $(DISTCLEAN_LIST)

# default rule
default: makedir all

# non-phony targets
$(TARGET).uf2: $(TARGET).elf
	$(UF) $(TARGET).elf $@
	
$(TARGET).elf: $(OBJ) $(LINK_FILE)
	$(LD) $(LDFLAGS) $(LINK_FILE) -o $@ $(OBJ)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	$(CC) $(CCOBJFLAGS) -o $@ $<

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.s
	$(AS) -o $@ $<

$(TARGET_DEBUG): $(OBJ_DEBUG)
	$(CC) $(CCFLAGS) $(DBGFLAGS) $(OBJ_DEBUG) -o $@

# phony rules
.PHONY: makedir
makedir:
	@mkdir -p $(BIN_PATH) $(OBJ_PATH)

.PHONY: all
all: $(TARGET).uf2


.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(CLEAN_LIST)

.PHONY: distclean
distclean:
	@echo CLEAN $(DISTCLEAN_LIST)
	@rm -f $(DISTCLEAN_LIST)