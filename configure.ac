AC_INIT([hildon-input-method-plugins], [3.9.73])

AM_INIT_AUTOMAKE

# Specify dir for local macros
AC_CONFIG_MACRO_DIR([m4])

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_MAKE_SET
AM_PROG_CC_STDC
AC_PROG_LIBTOOL
AC_HEADER_STDC
AM_GCONF_SOURCE_2

PKG_CHECK_MODULES(DEPS, gtk+-2.0 >= 2.4.0 gconf-2.0 > 2.0 glib-2.0 >= 2.20.3 x11 \
                        hildon-1 hildon-im-vkbrenderer3 hildon-input-method-framework-3.0 >= 2.1.44 \
                        hildon-input-method-ui-3.0 >= 2.0.4 imlayouts imengines-wp xkbfile xi)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

AC_OUTPUT([
	Makefile
	hildon-im-western-plugin-common3/Makefile
	hildon-im-fkb/Makefile
	hildon-im-keyboard-monitor/Makefile
	hildon-im-xkbtool/Makefile
	hildon-keyboard-assistant/Makefile
	data/Makefile
])
