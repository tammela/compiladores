#ifndef _UTIL_H
#define _UTIL_H

#define perr(msg, ...) { fprintf(stderr, msg, ##__VA_ARGS__); exit(-1); }

#endif
