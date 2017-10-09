.DEFAULT_GOAL= build

OBJDIR:= out
SRCDIR:= src
PROG:= monga-cc
BISON:= monga.y
LEX:= monga.l
SRCS:= monga.tab.c main.c lex.yy.c tree.c ptree.c symbols.c analyze.c

OBJS:= ${SRCS:%.c=${OBJDIR}/%.o}

UNAME:= $(shell uname -s)

CFLAGS= -g

ifeq ($(UNAME), Darwin)
LDFLAGS= -ll -llua -lm
else
LDFLAGS= -lfl -llua -lm
endif

${OBJDIR}:
	@mkdir $@

${OBJDIR}/%.o: ${SRCDIR}/%.c | ${OBJDIR}
	$(eval REALPATH:= $(realpath $<))
	${CC} -o $@ ${CFLAGS} -c ${REALPATH} --std=gnu99

${OBJDIR}/${PROG}: ${OBJS}
	${CC} -o $@ $^ ${LDFLAGS}

bison:
ifeq ($(UNAME), Darwin)
	(cd src && bison3 -d ${BISON})
else
	(cd src && bison -d ${BISON})
endif

lex:
	(cd src && lex --header-file=lex.yy.h ${LEX})

build: bison lex ${OBJDIR}/${PROG}

testes: build
	bash testes/testa.sh ${OBJDIR}/${PROG}

clean:
	rm -rf ${OBJDIR} ${SRCDIR}/lex.yy.* ${SRCDIR}/monga.output ${SRCDIR}/monga.tab.*

