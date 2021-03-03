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
	rm -rf ./build/bryant_src
	mkdir -p ./build
	cp -R variants/unpatched build/bryant_src
	cd build && tar jcf ../bryant_dist_src.tar.bz2 bryant_src
	cd build/bryant_src/build && tar jcf ../../../bryant_dist_bin.tar.bz2 bryant
