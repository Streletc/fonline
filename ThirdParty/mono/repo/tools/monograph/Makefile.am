
if HOST_WIN32
export HOST_CC
endif

glib_libs = $(top_builddir)/mono/eglib/libeglib.la

if DISABLE_EXECUTABLES
runtime_lib=$(top_builddir)/mono/mini/$(LIBMONO_LA) $(static_libs)
else
static_libs=	\
	$(top_builddir)/mono/metadata/libmonoruntimesgen.la	\
	$(top_builddir)/mono/sgen/libmonosgen.la	\
	$(top_builddir)/mono/utils/libmonoutils.la \
	$(glib_libs)

runtime_lib=$(static_libs)
endif

if DISABLE_EXECUTABLES
bin_PROGRAMS =
else
if DISABLE_LIBRARIES
bin_PROGRAMS =
else
if SUPPORT_SGEN
bin_PROGRAMS = monograph
endif
endif
endif

AM_CPPFLAGS = 				\
	-I$(top_srcdir)			\
	$(GLIB_CFLAGS)

monograph_LDADD = \
	$(runtime_lib)			\
	$(glib_libs)			\
	$(LLVM_LIBS)

if HOST_DARWIN
monograph_LDFLAGS=-framework CoreFoundation -framework Foundation
endif

GRAPHS=System.Object System.Enum System.Attribute System.ValueType System.Reflection.MemberInfo
OUT=$(GRAPHS:=.jpeg)

graphs: $(OUT)

%.jpeg: monograph
	./monograph -n -o $*.png corlib.dll $*
	convert -geometry '480x360>' $*.png $*.jpeg


