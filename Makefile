SRC_FILES=mm.c hash.c prime.c transform.c
OBJ_FILES=$(patsubst %.c, %.o, ${SRC_FILES})
DEP_FILES=$(patsubst %.c, %.dep, ${SRC_FILES})

VPATH=src

CFLAGS=-c -g
LDFLAGS=-g

appexp: ${OBJ_FILES}
	@echo "linking..."
	gcc -Wall -Wextra ${LDFLAGS} -o appexp ${OBJ_FILES}

%.o:%.c
	@echo "compiling..."
	gcc -Wall -Wextra ${CFLAGS} -o $@ $< 

.PHONY: clean
clean:
	rm *.o 
	rm appexp
	rm *.dep

include ${DEP_FILES}

%.dep: %.c
	@set -e; rm -f $@; \
	gcc -MM ${CFLAGS} $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

