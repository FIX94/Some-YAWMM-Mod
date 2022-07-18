
PREFIX = $(DEVKITPPC)/bin/powerpc-eabi-

AR = $(PREFIX)ar
AS = $(PREFIX)as
CC = $(PREFIX)gcc
CXX = $(PREFIX)g++
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy
RANLIB = $(PREFIX)ranlib
STRIP = $(PREFIX)strip

MACHDEP = -mcpu=750 -meabi -mhard-float
CFLAGS = $(MACHDEP) -O0 -s -Werror -Wall -fdata-sections -ffunction-sections
LDFLAGS = $(MACHDEP) -n -nostartfiles -nostdlib -Wl,--gc-sections,-T,openstub.ld -L.
ASFLAGS = -D_LANGUAGE_ASSEMBLY -DHW_RVL

TARGET_LINKED = patcher.elf
TARGET = ../../data/appboot.bin

CFILES = main.c utils.c loaddol.c loadelf.c
OBJS = main.o utils.o loaddol.o loadelf.o

DEPDIR = .deps

LIBS = 

all: $(TARGET)

%.o: %.s
	@$(CC) $(CFLAGS) $(DEFINES) $(ASFLAGS) -c $< -o $@

%.o: %.S
	@$(CC) $(CFLAGS) $(DEFINES) $(ASFLAGS) -c $< -o $@

%.o: %.c
	@$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

$(TARGET_LINKED): $(OBJS)
	@$(CC) -g -o $@ $(LDFLAGS) $(OBJS) $(LIBS)

$(TARGET): $(TARGET_LINKED)
	@$(OBJCOPY) -O binary -S $< $@

clean:
	@-$(RM) -rf $(TARGET_LINKED) $(OBJS) $(DEPDIR)
