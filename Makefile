DESTDIR ?= .
CXX ?= g++

test-shader:
	$(CXX) -o test-shader test-shader.cpp -lGLESv2 -lEGL $(LDFLAGS) $(CFLAGS)

install:
	install -m 0775 test-shader $(DESTDIR)/test-shader
    