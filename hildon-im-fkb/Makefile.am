AM_CPPFLAGS = $(DEPS_CFLAGS) -I$(top_srcdir)/include

himplugin_LTLIBRARIES = hildon_western_fkb.la

hildon_western_fkb_la_SOURCES = hildon-im-western-fkb.c
hildon_western_fkb_la_LDFLAGS = -module -avoid-version -Wl,--no-undefined
hildon_western_fkb_la_LIBADD = $(DEPS_LIBS) \
	$(top_srcdir)/shared/hildon-im-word-completer.lo \
	$(top_srcdir)/hildon-im-western-plugin-common3/libhildon-im-western-plugin-common.la
