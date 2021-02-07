</$objtype/mkfile

CFLAGS=-FTVw -p -Ishim -Ilua -Ilpeg -DLUA_USE_PLAN9

TARG=$O.lua

OBJS=lua.$O

LIBS=\
	lua/liblua.a$O\
	lpeg/liblpeg.a$O\
	shim/libshim.a$O

all:V: $TARG

clean:V:
	@{cd shim; mk clean}
	@{cd lua; mk clean}
	@{cd lpeg; mk clean}
	rm -f $TARG [$OS].out *.[$OS] *.a[$OS]

shim/libshim.a$O:
	@{cd shim; mk}

lua/liblua.a$O:
	@{cd lua; mk}

lpeg/liblpeg.a$O:
	@{cd lpeg; mk}

$TARG: $OBJS $LIBS
	$LD -o $TARG $prereq

%.$O: %.c
	$CC $CFLAGS $stem.c
