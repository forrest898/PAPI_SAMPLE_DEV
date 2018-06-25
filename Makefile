include Makefile.globals.inc

all:
		$(MAKE) -c src

clean: clean-local
		$(MAKE) -c src clean

clean-local:
		@- $(RM) *~
