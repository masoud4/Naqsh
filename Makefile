CC = clang
SRC = Naqsh.c
PKG = pkg-config --cflags --libs libswscale x11
DIS = -o Naqsh
install:
		${CC} ${SRC} -lm `${PKG}` ${DIS}

