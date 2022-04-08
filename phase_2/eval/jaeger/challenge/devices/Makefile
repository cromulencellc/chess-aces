all : hello-world hello-world-timer test-0 test-1 challenge

FORCE:

platform : FORCE
	mkdir -p build/o
	make -C platform

hello-world : platform FORCE
	make -C hello-world

hello-world-timer : platform FORCE
	make -C hello-world-timer

test-0 : platform FORCE
	make -C test-0

test-1 : platform FORCE
	make -C test-1

challenge : platform FORCE
	make -C challenge

clean :
	make -C platform clean
	make -C hello-world clean
	make -C hello-world-timer clean
	make -C test-0 clean
	make -C test-1 clean
	make -C challenge clean
	rm -rf build