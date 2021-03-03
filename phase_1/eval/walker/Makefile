.PHONY: clean variants

PKG_DIR ?= ./pkg
VARIANTS_DIR ?= ./variants

all_variants:
	for i in variants/*; do (cd $$i; make; cd ..); done

variants:
	docker run --rm -it -v `pwd`:/mnt aces_preproc aces_preproc -d /mnt/challenge -b /mnt/$(VARIANTS_DIR) --fix-chown

clean:
	rm -rf $(PKG_DIR) $(VARIANTS_DIR)

dist:
	rm -rf ./build/walker_src
	mkdir -p ./build
	cp -R variants/unpatched build/walker_src
	cd build && tar jcf ../walker_dist_src.tar.bz2 walker_src
	cd build/walker_src/build && tar jcf ../../../walker_dist_bin.tar.bz2 walker

