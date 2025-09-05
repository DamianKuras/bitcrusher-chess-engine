# OpenBench compatible wrapper makefile
.PHONY: all clean

EXE ?= Uci

all:
	cd .. && premake5 gmake --with-uci
	cd ../build && $(MAKE) Uci config=release_x64
	cp ../bin/Release/Uci $(EXE)

clean:
	cd build && $(MAKE) clean
	rm -f $(EXE)

