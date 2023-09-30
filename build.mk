.PHONY: all, clean

MAKEFILE := $(MAKE) -f Makefile
PWD := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
BUILD := $(PWD)/build

all: Makefile
	mkdir -p $(BUILD)
	$(MAKEFILE) CFLAGS=-DHAVE_INLINE
	$(MAKEFILE) install

configure: autogen.sh
	./autogen.sh

Makefile: configure
	./configure --enable-maintainer-mode --prefix $(BUILD)

clean: Makefile
	$(MAKEFILE) clean
