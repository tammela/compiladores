#ifndef _SYMBOLS_H
#define _SYMBOLS_H

void *findSymbol(char *id);
void addSymbol(Var *v);
void openBlock(void);
void closeBlock(void);
void initSymbolTable(void);

#endif
