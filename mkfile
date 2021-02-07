</$objtype/mkfile

CFLAGS=-FTVw -p -Ishim -Ilua -Ilpeg -DLUA_USE_PLAN9

TARG=$O.lua
BIN=/$objtype/bin

OBJS=lua.$O

LIBS=\
	lua/liblua.a$O\
	lpeg/liblpeg.a$O\
	shim/libshim.a$O

all:V: $TARG

clone:V:
	git/clone https://git.sr.ht/~kvik/lua9-lua lua
	git/clone https://git.sr.ht/~kvik/lua9-lpeg lpeg
	git/clone https://git.sr.ht/~kvik/lua9-shim shim

pull:V:
	@{cd lua; git/pull}
	@{cd lpeg; git/pull}
	@{cd shim; git/pull}

install:V: $TARG
	cp $TARG $BIN/lua

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
