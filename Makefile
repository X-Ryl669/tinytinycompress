CXXFLAGS=-fPIC -O0 -ggdb3

libT2C.o: T2C.hpp Compress.inc Decompress.inc
	$(CXX) $(CXXFLAGS) -c T2C.hpp -o $@

t2c: tests/main.cpp libT2C.o
	$(CXX) $(CXXFLAGS) -c tests/main.cpp -o tests/main.o
	$(CXX) $(CXXFLAGS) -o $@ tests/main.o

clean:
	-rm libT2C.o
	-rm t2c
