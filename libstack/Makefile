#CROSS = /mnt/disk2/opt/v5t_le/bin/arm_v5t_le-
AR=$(CROSS)ar
CC=$(CROSS)gcc

CFLAGS += -c -g -I ./inc -I ../libplatform/inc
LDFLAGS +=-L/usr/lib -lz -lm -L../libplatform/output -lplatform -lpthread


DIRS=src
SRCS=$(foreach DIR, $(DIRS), $(wildcard $(DIR)/*.c))
OBJS=$(patsubst %.c, %.o, $(SRCS))

LIB=./output/libStack.a

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) cru $(LIB) $(OBJS)

#$(BIN): $(LIB) $(TESTOBJS)
#	$(CROSS_CC) -o $@ $(TESTOBJS) $(LDFLAGS)

__install: $(LIB) $(BIN)
	#do nothing
	#cp $(BIN) $(BIN_DIR)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(OBJS) $(LIB) $(BIN)
