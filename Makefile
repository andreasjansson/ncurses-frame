LIBS = -lcurses -lm
CFLAGS = -Wall -g
LDFLAGS = ${LIBS}
CC = gcc

test: test.c frame.c libtermkey.a frame.h termkey.h
	${CC} -o test ${CFLAGS} ${LIBS} test.c frame.c libtermkey.a

sequencer: sequencer.c frame.c libtermkey.a frame.h termkey.h
	${CC} -o sequencer ${CFLAGS} ${LIBS} sequencer.c frame.c libtermkey.a
