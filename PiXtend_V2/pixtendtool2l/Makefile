PREFIX?=/usr/local
BIN=${DESTDIR}${PREFIX}/bin/


all:compile
${BIN}:
	mkdir -p ${@}

compile:pixtendtool2l

install:pixtendtool2l ${BIN}
	cp ${<} ${BIN}

pixtendtool2l:pixtendtool2l.c
		gcc -Wall -o "$@" "$<" -lpixtend -lwiringPi
