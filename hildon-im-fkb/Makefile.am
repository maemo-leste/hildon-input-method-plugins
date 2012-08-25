INCLUDES = \
       $(DEPS_CFLAGS)

libhildon_western_fkb_LTLIBRARIES = libhildon_western_fkb.la
libhildon_western_fkbdir = /usr/lib/hildon-input-method

libhildon_western_fkb_la_SOURCES =  hildon-im-word-completer.c hildon-im-western-fkb.c
libhildon_western_fkb_la_CFLAGS = $(HILDONINPUPMETHODPLUGINS_CFLAGS) -I$(srcdir)/../include
libhildon_western_fkb_la_LDFLAGS = -avoid-version
libhildon_western_fkb_la_LIBADD = $(DEPS_LIBS) -L/usr/lib -limengines-wp -limlayouts \
            ../hildon-im-western-plugin-common3/libhildon-im-western-plugin-common.la
