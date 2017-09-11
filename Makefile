.DEFAULT_GOAL= build

OBJDIR:= out
SRCDIR:= src
PROG:= monga-cc
LEXINPUT:= monga.l
SRCS:= monga.tab.c main.c lex.yy.c

OBJS:= ${SRCS:%.c=${OBJDIR}/%.o}

CFLAGS= -g
LDFLAGS= -lfl

${OBJDIR}:
	@mkdir $@

${OBJDIR}/%.o: ${SRCDIR}/%.c | ${OBJDIR}
	$(eval REALPATH:= $(realpath $<))
	${CC} -o $@ ${CFLAGS} -c ${REALPATH}

${OBJDIR}/${PROG}:
	${CC} -o $@ ${OBJS} ${LDFLAGS}

bison:
	@(if [ -d ${OBJDIR} ]; then rm -rf ${OBJDIR}; fi && \
	   cd src && bison -d monga.y)

lex:
	@(if [ -d ${OBJDIR} ]; then rm -rf ${OBJDIR}; fi && \
		cd src && lex --header-file=lex.yy.h ${LEXINPUT})

build: bison lex ${OBJS} ${OBJDIR}/${PROG}

testes: build
	bash testes/testa.sh ${OBJDIR}/${PROG}

clean:
	rm -rf ${OBJDIR}

