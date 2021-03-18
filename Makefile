CC := gcc
SRCD := src
BLDD := build
INCD := include
TFLD := testFiles

MAIN  := $(BLDD)/main.o

ALL_SRCF := $(shell find $(SRCD) -type f -name *.c)
ALL_OBJF := $(patsubst $(SRCD)/%,$(BLDD)/%,$(ALL_SRCF:.c=.o))
ALL_FUNCF := $(filter-out $(MAIN) $(AUX), $(ALL_OBJF))

INC := -I $(INCD)

CFLAGS := -g -O2 -Wall -Werror -Wno-unused-variable -Wno-unused-function -MMD
COLORF := -DCOLOR
DFLAGS := -g -DDEBUG -DCOLOR
PRINT_STAMENTS := -DERROR -DSUCCESS -DWARN -DINFO

STD := -std=gnu11
LIBS := -lm

CFLAGS += $(STD)

EXEC := lkmalloc

.PHONY: clean all setup debug

all: setup $(EXEC) 

debug: CFLAGS += $(DFLAGS) $(PRINT_STAMENTS) $(COLORF)
debug: all

setup: $(BLDD)

$(BLDD):
		mkdir -p $(BLDD)

$(EXEC): $(ALL_OBJF)
		$(CC) $^ -o $@ $(LIBS)

$(BLDD)/%.o: $(SRCD)/%.c
		$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
		rm -rf $(BLDD)
		rm $(EXEC)

wipe:
		rm -rf $(BLDD)
		rm -rf $(EXEC)



.PRECIOUS: $(BLDD)/*.d
-include $(BLDD)/*.d