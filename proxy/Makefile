#CROSS = /mnt/disk2/opt/v5t_le/bin/arm_v5t_le-
#CROSS = i386-mandrake-linux-
AR=$(CROSS)ar
CC=$(CROSS)gcc

CFLAGS += -c -g -I ./inc -I ../libplatform/inc -I../database/inc
#LDFLAGS += -lmygcc -L/usr/lib -lz -lm -L../libplatform/output -lplatform -L../database/output -ldatabase  -L/usr/lib/mysql -lmysqlclient -lpthread
LDFLAGS += -L/usr/lib -lz -lm -L../libplatform/output -lplatform -L../database/output -ldatabase  -L/usr/lib/mysql -lmysqlclient -lpthread

DIRS=.
SRCS=$(foreach DIR, $(DIRS), $(wildcard $(DIR)/*.c))
OBJS=$(patsubst %.c,%.o, $(SRCS))

EXE=go2see

all:$(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(LDFLAGS)

#$(BIN): $(LIB) $(TESTOBJS)
#	$(CROSS_CC) -o $@ $(TESTOBJS) $(LDFLAGS)

__install: $(LIB) $(BIN)
	#do nothing
	#cp $(BIN) $(BIN_DIR)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(OBJS) $(LIB) $(EXE)
