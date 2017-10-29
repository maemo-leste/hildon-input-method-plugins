INCLUDES = \
       $(DEPS_CFLAGS)

hildon_im_keyboard_monitor_LTLIBRARIES = hildon_im_keyboard_monitor.la
hildon_im_keyboard_monitordir = /usr/lib/hildon-input-method

hildon_im_keyboard_monitor_la_SOURCES = $(srcdir)/../shared/hildon-im-xkb.c hildon-im-keyboard-monitor.c
hildon_im_keyboard_monitor_la_CFLAGS = $(HILDONINPUPMETHODPLUGINS_CFLAGS) -I$(srcdir)/../include
hildon_im_keyboard_monitor_la_LDFLAGS = -module -avoid-version
hildon_im_keyboard_monitor_la_LIBADD = $(DEPS_LIBS) \
            ../hildon-im-western-plugin-common3/libhildon-im-western-plugin-common.la
