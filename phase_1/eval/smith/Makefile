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
	rm -rf ./build/smith_src
	mkdir -p ./build
	cp -R variants/unpatched build/smith_src
	cd build && tar jcf ../smith_dist_src.tar.bz2 smith_src
	cd build/smith_src/build && tar jcf ../../../smith_dist_bin.tar.bz2 smith

