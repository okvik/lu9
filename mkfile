</$objtype/mkfile

CFLAGS=-FTVw -p -Ilua/shim -Ilua -Ilpeg -DLUA_USE_PLAN9

TARG=$O.lu9
BIN=/$objtype/bin
MOD=/sys/lib/lua
MAN=/sys/man

OBJS=lu9.$O

LIBS=\
	lua/liblua.a.$O\
	lpeg/liblpeg.a.$O\
	p9/libp9.a.$O

all:V: $TARG

clone:V:
	git/clone https://git.sr.ht/~kvik/lu9-lua lua
	git/clone https://git.sr.ht/~kvik/lu9-lpeg lpeg
	git/clone https://git.sr.ht/~kvik/lu9-p9 p9

pull:V:
	@{cd lua; git/pull}
	@{cd lpeg; git/pull}
	@{cd p9; git/pull}

install:V: $TARG $MAN/1/lu9
	luav=`{$TARG -v}
	cp $TARG $BIN/lu9
	cp $TARG $BIN/lu9-^$luav
	mkdir -p $MOD/$luav

clean:V:
	@{cd lua; mk clean}
	@{cd lpeg; mk clean}
	@{cd p9; mk clean}
	rm -f $TARG [$OS].out *.[$OS] *.a.[$OS]

lua/liblua.a.$O:
	@{cd lua; mk}

lpeg/liblpeg.a.$O:
	@{cd lpeg; mk}
	
p9/libp9.a.$O:
	@{cd p9; mk}
	
$MAN/1/%: %.man
	cp $stem.man $target

$TARG: $OBJS $LIBS
	$LD -o $TARG $prereq

%.$O: %.c
	$CC $CFLAGS $stem.c
