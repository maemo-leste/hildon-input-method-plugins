AM_CPPFLAGS = $(DEPS_CFLAGS)

himplugin_LTLIBRARIES = hildon_keyboard_assistant.la

hildon_keyboard_assistant_la_SOURCES =  $(srcdir)/../shared/hildon-im-word-completer.c hildon-keyboard-assistant.c
hildon_keyboard_assistant_la_CFLAGS = $(HILDONINPUPMETHODPLUGINS_CFLAGS) -I$(srcdir)/../include
hildon_keyboard_assistant_la_LDFLAGS = -module -avoid-version
hildon_keyboard_assistant_la_LIBADD = $(DEPS_LIBS) \
            ../hildon-im-western-plugin-common3/libhildon-im-western-plugin-common.la
