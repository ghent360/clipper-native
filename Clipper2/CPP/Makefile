%.bc : %.cpp
	emcc -std=c++11 -s WASM=1 -O2 -o $@ -c $<

clipper_js.js : clipper.bc clipper_js.bc
	emcc --bind -s WASM=1 -O2 -o $@ $?	

all: clipper_js.js
