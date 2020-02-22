AM_CPPFLAGS = $(DEPS_CFLAGS) -I$(top_srcdir)/include

himplugin_LTLIBRARIES = hildon_keyboard_assistant.la

hildon_keyboard_assistant_la_SOURCES = hildon-keyboard-assistant.c
hildon_keyboard_assistant_la_LDFLAGS = -module -avoid-version -Wl,--no-undefined
hildon_keyboard_assistant_la_LIBADD = $(DEPS_LIBS) \
	$(top_srcdir)/shared/hildon-im-word-completer.lo \
	../hildon-im-western-plugin-common3/libhildon-im-western-plugin-common.la
