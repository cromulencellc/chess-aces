.PHONY: clean variants

PKG_DIR ?= ./pkg
VARIANTS_DIR ?= ./variants

generate_variants:
	docker run --rm -it -v `pwd`:/mnt aces_preproc aces_preproc -d /mnt/challenge -b /mnt/$(VARIANTS_DIR) --fix-chown

build_variants:
	for i in variants/*; do (cd $$i; make -j8; cd ..); done
