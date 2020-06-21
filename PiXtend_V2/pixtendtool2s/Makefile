PREFIX?=/usr/local

BIN=${DESTDIR}${PREFIX}/bin/

all:compile

compile:pixtendtool2s
${BIN}:
	mkdir -p ${@}
	
install:pixtendtool2s ${BIN}
	cp ${<} ${BIN}

pixtendtool2s:pixtendtool2s.c
		gcc -Wall -o "$@" "$<" -lpixtend -lwiringPi
