SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

CFLAGS = -std=c11 -Wall -Wextra -pedantic -O0 -g3 -I .


default: pm


RDESC_DIR := ../..
RDESC_FEATURES := full
RDESC_MODE := debug

include $(RDESC_DIR)/rdesc.mk

# rdesc public headers
rdesc:
	ln -s $(RDESC_INCLUDE_DIR) rdesc


%.o: %.c rdesc
	$(CC) $(CFLAGS) -MMD -MP -c $<

pm: $(OBJS) $(RDESC)
	$(CC) $(CFLAGS) $^ -o $@


clean:
	$(RM) *.o *.d pm rdesc

.PHONY: clean clean-vendor default


-include $(OBJS:.o=.d)
