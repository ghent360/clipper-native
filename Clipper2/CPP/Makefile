%.bc : %.cpp
	emcc -std=c++11 -s WASM=1 -O3 -o $@ -c $<

clipper_js.js : clipper.bc clipper_js.bc
	emcc --bind -s WASM=1 -O3 -o $@ clipper.bc clipper_js.bc
	mv $@ $@.tmp
	sed "s/module\[[\'\"]exports[\'\"]\]/module[\"exports_\"]/g" < $@.tmp >$@.tmp1
	cat index.head.js $@.tmp1 >$@
	rm $@.tmp $@.tmp1

clipper.cpp: clipper.h

clipper_js.cpp: clipper.h

all: clipper_js.js
