PREFIX?=/usr/local

LIB=${DESTDIR}${PREFIX}/lib/
INCLUDE=${DESTDIR}${PREFIX}/include/

all:compile

compile:libpixtend.a

${LIB} ${INCLUDE}:
	mkdir -p ${@}
	
pixtend.o:pixtend.c pixtend.h
	gcc -c ${<}

libpixtend.a:pixtend.o
	ar crs ${@} ${<}
	ranlib ${@}


install:${LIB} ${INCLUDE}
	cp libpixtend.a ${LIB}
	cp pixtend.h ${INCLUDE}
	
