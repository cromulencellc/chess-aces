all : acd2 fsd maint rcd

FORCE:

universal : FORCE
	make -C universal

acd2 : universal FORCE
	make -C acd2

fsd : universal FORCE
	make -C fsd

maint : universal FORCE
	make -C maint

rcd : universal FORCE
	make -C rcd

clean :
	make -C universal clean
	make -C acd2 clean
	make -C maint clean
	make -C rcd clean
	make -C fsd clean