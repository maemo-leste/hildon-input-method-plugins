INCLUDES = \
       $(DEPS_CFLAGS)

bin_PROGRAMS = hildon-im-xkbtool

hildon_im_xkbtool_SOURCES = $(srcdir)/../shared/hildon-im-xkb.c hildon-im-xkbtool.c
hildon_im_xkbtool_CFLAGS = $(HILDONINPUPMETHODPLUGINS_CFLAGS) -I$(srcdir)/../include
hildon_im_xkbtool_LDADD = $(DEPS_LIBS)
