
PROJECT_NAME = mouse

INCLUDES = -I include -I include/rtt

SRC_FILES = src/$(PROJECT_NAME).c \
			src/startup.c \
			src/utils.c \
			src/spi.c \
			src/paw3395.c \
			src/delay.c \

LINKER_SCRIPT = nrf52820.ld

#########################
## configure toolchain ##
#########################

CROSS_COMPILE  = arm-none-eabi
CC            := $(CROSS_COMPILE)-gcc

CFLAGS += $(INCLUDES)
CFLAGS += -std=gnu23
CFLAGS += -mcpu=cortex-m4 -ffreestanding -nostdlib
CFLAGS += -Wall -Wextra
OFLAGS ?= -Og -g3 -flto
CFLAGS += $(OFLAGS)

LDFLAGS += -T $(LINKER_SCRIPT)

CFLAGS += -DDBG=0

###########
## build ##
###########

BUILDDIR = build
ELF  := $(BUILDDIR)/$(PROJECT_NAME).elf
BIN  := $(BUILDDIR)/$(PROJECT_NAME).bin
MAP  := $(BUILDDIR)/$(PROJECT_NAME).map
LIST := $(BUILDDIR)/$(PROJECT_NAME).list

all: $(ELF)

dbg: OFLAGS = -Og -g3 -flto
dbg: clean $(ELF)

rel: OFLAGS = -Os -flto
rel: clean $(ELF)

$(ELF): $(SRC_FILES)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -Wl,-Map=$(MAP) -Wl,--print-memory-usage $^ -o $@
	$(CROSS_COMPILE)-objcopy -O binary $(ELF) $(BIN)
	$(CROSS_COMPILE)-objdump -h -d $(ELF) > $(LIST)
	$(CROSS_COMPILE)-size $(ELF)

clean:
	rm -rf $(BUILDDIR)
