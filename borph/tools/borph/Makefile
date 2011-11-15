###################
# BORPH utilities #
###################

CC := gcc

CFLAGS += -Wall -I$(INCLUDES)
ifeq ($(DEBUG),y)
  CFLAGS += -g -DDEBUG 
else
  CFLAGS += -O2
endif

all: mkbof showbof

mkbof: mkbof.c
	$(CC) $(CFLAGS) -o mkbof mkbof.c -Iinclude -I../../include

showbof: showbof.c
	$(CC) $(CFLAGS) -o showbof showbof.c -Iinclude -I../../include

clean:
	rm -rf mkbof showbof

