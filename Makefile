PREFIX = /usr/local
CC = gcc
INCLUDES = includes/
CFLAGS = -O3 -std=c++17 -Wno-unused-result
TEST_FLAGS = -Og -std=c++17
LIBS = -lstdc++
SRC = src

gdec++.so: source/gdec++.cpp ${INCLUDES}*
	${CC} -I${INCLUDES} ${CFLAGS} -fPIC -shared -c $< -o lib/$@ ${LIBS}
	${CC} -I${INCLUDES} ${TEST_CFLAGS} -fPIC -shared -c $< -o lib/debug_gdec.so ${LIBS}

test: test/test.cpp gdec++.so
	${CC} ${TEST_FLAGS} ${INCLUDES} -fPIC -shared -c src/gdec++.cpp -o test/gdec_debug.so ${LIBS}

install: gdec++.so
	cp lib/gdec++.so ${PREFIX}/lib
	cp includes/gdec++.hpp ${PREFIX}/include

.PHONY: clean test

uninstall:
	rm ${PREFIX}/lib/gdec++.so
	rm ${PREFIX}/include/gdec++.hpp

clean:
	rm lib/gdec++.so lib/debug_gdec.so


