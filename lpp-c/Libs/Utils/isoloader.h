#include <stdio.h>
#include <stdarg.h>
#include "csoread.h"

#define MAKE_JUMP(f) (0x08000000 | (((u32)(f) >> 2)  & 0x03ffffff))
#define ISO_MAX_FILES 1024
#define ISO_MAX_DIRS 256

typedef struct File {
	char path[1024];
	char name[256];
	u32 addr;
	u32 size;
	u32 fd;
} File;

void IsoOpen(FILE* fd);
File* GetFile(char* fullpath);
void ReadFile(File* file, void* buf);
