sources=(wildcard src/*.cpp)
libs=($sources:.cpp=.so)

hooks: ($libs)
	mv src/*.so bin/.

%.so: %.cpp
	g++ -fPIC --shared $< -o $@

