%.bc : %.cpp
	emcc -std=c++11 -s TOTAL_MEMORY=256MB -s WASM=1 -O3 -o $@ -c $<

all: clipper_js.js

build.js:
	date -u +"module.exports={build:'%g%m%d.%H%M'};" >$@

clipper_js.js : clipper.bc clipper_js.bc clipper_offset.bc clipper_offset_js.bc build.js
	emcc --bind -s TOTAL_MEMORY=256MB -s WASM=1 -O3 -o $@ clipper.bc clipper_js.bc clipper_offset.bc clipper_offset_js.bc
	mv $@ $@.tmp
	sed "s/module\[[\'\"]exports[\'\"]\]/module[\"exports_\"]/g" < $@.tmp >$@.tmp1
	cat index.head.js $@.tmp1 >$@
	rm $@.tmp $@.tmp1

clipper.cpp: clipper.h

clipper_js.cpp: clipper.h

clipper_offset.cpp: clipper_offset.h clipper.h clipper.cpp

clipper_offset_js.cpp: clipper_offset.h clipper.h