bin_PROGRAMS = uart

uart_SOURCES = main.c 
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
