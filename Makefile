# tool macros
AS = arm-none-eabi-as
ASFLAGS := -g
CC := arm-none-eabi-gcc
CCFLAGS := -mthumb -mcpu=cortex-m0plus -nostdlib -ggdb -ffreestanding -std=c11
CCOBJFLAGS := $(CCFLAGS) -c
LD = arm-none-eabi-ld
LDFLAGS =  --verbose  -T
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
SRC_C := $(shell cd src && find . -name "*.c")
SRC_S := $(shell cd src && find . -name "*.s")
OBJ_C := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(basename $(SRC_C))))
OBJ_S := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(basename $(SRC_S))))
OBJ := $(foreach x, $(notdir $(OBJ_C) $(OBJ_S)), $(addprefix $(OBJ_PATH)/, $(x)))


# clean files list
CLEAN_LIST := $(TARGET) \
			  $(DISTCLEAN_LIST)

# default rule
default: makedir all

# non-phony targets
$(TARGET).uf2: $(TARGET).elf
	$(UF) $(TARGET).elf $@
# $(LD) $(LDFLAGS) $(LINK_FILE) -o $@ $(OBJ)
$(TARGET).elf: $(OBJ_C) $(OBJ_S) $(LINK_FILE)
	$(CC) $(CCFLAGS) -lc -o $@ $(OBJ) -T $(LINK_FILE) -lgcc  -specs=nano.specs

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	$(CC) $(CCOBJFLAGS) -o $(OBJ_PATH)/$(notdir $@) $<

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.s
	$(AS) $(ASFLAGS) -o $(OBJ_PATH)/$(notdir $@) $<


# phony rules
.PHONY: makedir
makedir:
	@mkdir -p $(BIN_PATH) $(OBJ_PATH)

.PHONY: all
all: $(TARGET).uf2

.PHONY: upload
upload: $(TARGET).uf2
	@openocd -f interface/picoprobe.cfg -f target/rp2040.cfg -c "program bin/app.elf verify reset exit"


.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(CLEAN_LIST)
