include Makefile.globals.inc

all:
		$(MAKE) -C src

clean: clean-local
		$(MAKE) -C src clean

clean-local:
		@- $(RM) *~
		@- $(RM) wowie*
