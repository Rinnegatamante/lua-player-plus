#ifndef __LIBUNRAR_H
#define __LIBUNRAR_H

#ifdef __cplusplus
extern "C" {
#endif

/*
Results:
0: OK
1: File doesnt exists
2: File corrupt/Not rar
3: Wrong password
*/

int mainRAR(int argc, char *argv[]);
int rarExtract(const char *rarfile,const char *extDir,const char *pass);

#ifdef __cplusplus
}
#endif

#endif
