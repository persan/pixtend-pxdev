
PREFIX?=/usr/local
BIN=${DESTDIR}${PREFIX}/bin/

all:compile
compile:pxauto2l

${BIN}:
	mkdir -p ${@}
	
install:pxauto2l ${BIN}
	cp ${<} ${BIN}

pxauto2l:pxauto2l.c  pxauto2l.h
		gcc -Wall -o "$@" "$<" -lmenu -lform -lpanel -lncurses -lrt -lpixtend -lwiringPi

