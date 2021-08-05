</$objtype/mkfile

VERSION=5.4

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

pull:V:
	@{if(test -d lua){cd lua; git/pull}
	  if not git/clone https://git.sr.ht/~kvik/lu9-lua lua}
	@{if(test -d lpeg){cd lpeg; git/pull}
	  if not git/clone https://git.sr.ht/~kvik/lu9-lpeg lpeg}
	@{if(test -d p9){cd p9; git/pull}
	  if not git/clone https://git.sr.ht/~kvik/lu9-p9 p9}

install:QV: $TARG $MAN/1/lu9
	cp $TARG $BIN/lu9
	cp $TARG $BIN/lu9-^$VERSION
	mkdir -p $MOD/$VERSION

clean:QV:
	@{cd lua; mk clean}
	@{cd lpeg; mk clean}
	@{cd p9; mk clean}
	rm -f [$OS].* *.[$OS] *.a.[$OS]

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
