INCLUDES = \
       $(DEPS_CFLAGS)

himplugin_LTLIBRARIES = hildon_western_fkb.la

hildon_western_fkb_la_SOURCES =  $(srcdir)/../shared/hildon-im-word-completer.c hildon-im-western-fkb.c
hildon_western_fkb_la_CFLAGS = $(HILDONINPUPMETHODPLUGINS_CFLAGS) -I$(srcdir)/../include
hildon_western_fkb_la_LDFLAGS = -module -avoid-version
hildon_western_fkb_la_LIBADD = $(DEPS_LIBS) \
            ../hildon-im-western-plugin-common3/libhildon-im-western-plugin-common.la
