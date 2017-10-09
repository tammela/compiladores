.DEFAULT_GOAL= build

OBJDIR:= out
SRCDIR:= src
PROG:= monga-cc
LEXINPUT:= monga.l
SRCS:= analyze.c symbols.c tree.c monga.tab.c main.c lex.yy.c ptree.c \
       util.c geracod.c

OBJS:= ${SRCS:%.c=${OBJDIR}/%.o}

UNAME:= $(shell uname -s)

CFLAGS= -g -Wall

ifeq ($(UNAME), Darwin)
LDFLAGS= -ll -llua -lm
else
LDFLAGS= -lfl -llua -lm
endif

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
