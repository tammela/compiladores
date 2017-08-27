.DEFAULT_GOAL= build

OBJDIR:= output
PROG:= monga-cc
LEXINPUT:= mongalang.lex
SRCS:= main.c

OBJS:= ${SRCS:%.c=${OBJDIR}/%.o}

CFLAGS=
LDFLAGS=

${OBJDIR}:
	mkdir $@

${OBJDIR}/%.o: %.c | ${OBJDIR}
	$(eval REALPATH:= $(realpath $<))
	${CC} -o $@ ${CFLAGS} -c ${REALPATH}

${PROG}: ${OBJS}
	${CC} -o $@ ${OBJS} ${LDFLAGS}

${GENLEX}:
	lex --header-file=lex.yy.h ${LEXINPUT}

build: ${GENLEX} ${PROG}

clean:
	rm -rf ${OBJDIR}

