CC = gcc
CFLAGS = -fPIC -std=c99 -Wall -Werror

SRC = $(wildcard *.c)
HDR = $(wildcard *.h)
OBJS = ${SRC:.c=.o}
LIB = libxpb

VERSION_MAJ = 0
VERSION_MIN = 2
VERSION_REV = 0

SONAME = ${LIB}.so.${VERSION_MAJ}
LIBNAME = ${SONAME}.${VERSION_MIN}.${VERSION_REV}

LIBFLAGS = -shared -Wl,-soname,${SONAME} -lX11

INSTALL_PATH = /usr/local
INSTALL_PATH_HDR = ${INSTALL_PATH}/include
INSTALL_PATH_LIB = ${INSTALL_PATH}/lib

.PHONY: all
all: ${LIB}

${LIB}: ${OBJS}
	${CC} ${LIBFLAGS} -o ${LIBNAME} ${OBJS} 

.PHONY: install
install: all install_lib install_hdr

.PHONY: install_lib
install_lib:
	@echo Installing libxpb to ${INSTALL_PATH_LIB}
	@mkdir -p ${INSTALL_PATH_LIB}
	@cp -f ${LIBNAME} ${INSTALL_PATH_LIB}
	@chmod 755 ${INSTALL_PATH_LIB}/${LIBNAME}

.PHONY: install_hdr
install_hdr:
	@echo Installing headers to ${INSTALL_PATH_HDR}
	@mkdir -p ${INSTALL_PATH_HDR}
	@for file in ${HDR}; do \
		cp -f $$file ${INSTALL_PATH_HDR}; \
		chmod 644 ${INSTALL_PATH_HDR}/$$file; \
	done

.PHONY: clean
clean:
	@rm -f ${LIBNAME} ${OBJS}
