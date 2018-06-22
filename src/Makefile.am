bin_PROGRAMS = uart

uart_SOURCES = main.c      uart.h      uart-config.h uart-config.c \
			   uart-init.h uart-init.c uart-share.h  uart-share.c\
			   uart-read.h uart-read.c uart-text.h   uart-text.c \
			   uart-receive.h uart-receive.c uart-write.h uart-write.c\
			   uart-send.h uart-send.c uart-bottom.h uart-bottom.c 

uart_CFLAGS = \
    $(GLIB_CFLAGS)  \
    $(GTK_CFLAGS)  \
	$(VTE_CFLAGS)
uart_LDADD = \
    $(GLIB_LIBS)    \
    $(GTK_LIBS)    \
	$(VTE_LIBS)
uart_VALAFLAGS = \
	--pkg posix \
	--pkg gtk+-3.0 \
	--target-glib=$(GLIB_REQUIRED) 
-include $(top_srcdir)/git.mk
