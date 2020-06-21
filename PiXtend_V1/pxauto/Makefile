PREFIX?=/usr/local
BIN=${DESTDIR}${PREFIX}/bin/
LIB=${DESTDIR}${PREFIX}/lib/

all:compile
${BIN}:
	mkdir -p ${@}

compile:pxauto
install:pxauto ${BIN}
	cp ${<} ${DESTDIR}${PREFIX}/bin/

pxauto:pxauto.c
	gcc -Wall -o "$@" "$<" -lmenu -lform -lpanel -lncurses -lrt -lpixtend -lwiringPi
