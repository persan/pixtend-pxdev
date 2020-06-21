PREFIX?=/usr/local
all:compile
compile:
	${MAKE} -C pixtend    $@
	${MAKE} -C PiXtend_V1 $@
	${MAKE} -C PiXtend_V2 $@

%:
	${MAKE} -C pixtend    $@
	${MAKE} -C PiXtend_V1 $@
	${MAKE} -C PiXtend_V2 $@
	
