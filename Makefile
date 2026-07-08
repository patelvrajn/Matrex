# To compile using all available cores use: -j$(shell nproc)
# Only compile using 1 core because of memory consumption.
MAKEFLAGS += -j1

.PHONY: all release debug tests clean

all: release debug tests

release:
	$(MAKE) -C source

debug:
	$(MAKE) -C source DEBUG=1

tests:
	$(MAKE) -C tests

clean:
	$(MAKE) -C source clean
	$(MAKE) -C tests clean