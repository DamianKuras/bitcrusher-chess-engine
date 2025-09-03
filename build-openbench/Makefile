# OpenBench compatible wrapper makefile
.PHONY: all clean

# Default values
EXE ?= Uci
CONFIG ?= release_x64

all:
	cd .. && premake5 gmake --with-uci
	cd ../build && $(MAKE) Uci config=$(CONFIG)
	cp ../bin/Debug/Uci $(EXE)

clean:
	cd build && $(MAKE) clean
	rm -f $(EXE)

%:
	premake5 gmake --with-uci
	cd build && $(MAKE) Uci config=$(CONFIG)
	cp bin/Debug/Uci $@
