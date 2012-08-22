INCLUDES = \
       $(DEPS_CFLAGS)

lib_LTLIBRARIES = hildon_western_fkb.la
libdir = /usr/lib/hildon-input-method

hildon_western_fkb_la_SOURCES =  hildon-im-word-completer.c hildon-im-western-fkb.c
hildon_western_fkb_la_CFLAGS = $(HILDONINPUPMETHODPLUGINS_CFLAGS) -I$(srcdir)/../include
hildon_western_fkb_la_LDFLAGS = -module -avoid-version
hildon_western_fkb_la_LIBADD = $(DEPS_LIBS) -L/usr/lib -limlayouts
