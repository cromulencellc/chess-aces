all : lib test-0 test-1 challenge maintenance_test

FORCE:

lib: FORCE
	make -C lib

test-0: lib FORCE
	make -C test-0

test-1: lib FORCE
	make -C test-1

challenge: lib FORCE
	make -C challenge

maintenance_test: lib FORCE
	make -C maintenance_test

clean :
	make -C lib clean
	make -C test-0 clean
	make -C test-1 clean
	make -C challenge clean
	make -C maintenance_test clean