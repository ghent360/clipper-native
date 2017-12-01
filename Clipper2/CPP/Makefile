%.bc : %.cpp
	emcc -std=c++11 -O2 -o $@ -c $<

clipper_js.js : clipper.bc clipper_js.bc
	emcc --bind -O2 -o $@ clipper.bc clipper_js.bc

clipper.cpp: clipper.h

clipper_js.cpp: clipper.h

all: clipper_js.js
