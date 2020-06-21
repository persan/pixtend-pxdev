PREFIX?=/usr/local
BIN=${DESTDIR}${PREFIX}/bin/

all:compile
compile:pixtendtool
${BIN}:
	mkdir -p ${@}

install:pixtendtool ${BIN}
	cp $< ${BIN}
 
pixtendtool:pixtendtool.c
	gcc -Wall -o "$@" "$<" -lpixtend -lwiringPi
