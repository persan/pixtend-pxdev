PREFIX?=/usr/local

BIN=${DESTDIR}${PREFIX}/bin/

all:compile

compile:pxauto2s

${BIN}:
	mkdir -p ${@}

install:pxauto2s ${BIN}
	cp ${<} ${BIN}

pxauto2s:pxauto2s.c  pxauto2s.h
		gcc -Wall -o "$@" "$<" -lmenu -lform -lpanel -lncurses -lrt -lpixtend -lwiringPi
