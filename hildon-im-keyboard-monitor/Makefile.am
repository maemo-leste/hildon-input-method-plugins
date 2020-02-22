AM_CPPFLAGS = $(DEPS_CFLAGS) -I$(top_srcdir)/include

himplugin_LTLIBRARIES = hildon_im_keyboard_monitor.la

hildon_im_keyboard_monitor_la_SOURCES = hildon-im-keyboard-monitor.c
hildon_im_keyboard_monitor_la_LDFLAGS = \
	-module -avoid-version -Wl,--no-undefined
hildon_im_keyboard_monitor_la_LIBADD = $(DEPS_LIBS) \
	../hildon-im-western-plugin-common3/libhildon-im-western-plugin-common.la \
	$(top_srcdir)/shared/hildon-im-xkb.lo
