#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "isofs_driver.h"
#include "isoloader.h"

int IsofsReadSectors(int lba, int nsectors, void *buf, int *eod);

#define MAX_HANDLERS	33
#define SIZE_OF_SECTORS	(SECTOR_SIZE*8)+64

Iso9660DirectoryRecord main_record;
FileHandle handlers[MAX_HANDLERS];//0x0000A7C0
u8 *sectors;

SceUID isofs_sema;//0x0000A798
int g_lastLBA, g_lastReadSize;
int increased = 0;

int iso_size = 0;
int umd_is_cso = 0;
extern FILE* umdfd;
#define SECTOR_SIZE 0x800
#define SIZE_OF_SECTORS	(SECTOR_SIZE*8)+64
#define DebugPrintf printf
Iso9660DirectoryRecord main_record;

int ReadSectors(int lba, int nsectors, void* buf){
	if(umd_is_cso){
		return CisofileReadSectors(lba, nsectors, buf, NULL);
	}else{
		fseek(umdfd, lba*0x800, SEEK_SET);
		return fread(buf, 1, nsectors*0x800, umdfd);
	}
	return 0;
}

u32 IsoSize(){
	u32 size = 0;
	fseek(umdfd, 0, SEEK_END);
	size = ftell(umdfd);
	fseek(umdfd, 0, SEEK_SET);
	return size;
}

int CisoOpen(FILE*);
void IsoOpen(FILE* fd){
	sectors = malloc(SIZE_OF_SECTORS);
	umdfd = fd;
	iso_size = IsoSize();
	if(CisoOpen(fd) == 0){
		umd_is_cso = 1;
	}

	memset(sectors, 0, SIZE_OF_SECTORS);
	IsofsReadSectors(0x10, 1, sectors, NULL);
	memcpy(&main_record, sectors+0x9C, sizeof(Iso9660DirectoryRecord));
}

void ReadFile(File* file, void* buf){
	memset(buf, 0, file->size);
	ReadSectors(file->addr/0x800, file->size<0x800?1:((file->size/0x800)+1), buf);
}

int IsofsReadSectors(int lba, int nsectors, void *buf, int *eod)
{
	if (buf == sectors)
	{
		if (nsectors > 8)
		{
			return -1;
		}

		memset(sectors+(nsectors*SECTOR_SIZE), 0, 64);
	}

	return ReadSectors(lba, nsectors, buf);
}

//0x000027A8
inline int SizeToSectors(int size)
{
	int len = size / SECTOR_SIZE;

	if ((size % SECTOR_SIZE) != 0)
	{
		len++;
	}

	return len;
}

//0x00002780
void UmdNormalizeName(char *filename)
{
	char *p = strstr(filename, ";1");

	if (p)
	{
		*p = 0;
	}
}

//0x00002818
int GetPathAndName(char *fullpath, char *path, char *filename)
{
	char fullpath2[256];

	strcpy(fullpath2, fullpath);

	if (fullpath2[strlen(fullpath2)-1] == '/')
	{
		fullpath2[strlen(fullpath2)-1] = 0;
	}

	char *p = strrchr(fullpath2, '/');
	
	if (!p)
	{
		if (strlen(fullpath2)+1 > 32)
		{
			/* filename too big for ISO9660 */
			return -1;
		}

		memset(path, 0, 256);
		UmdNormalizeName(fullpath2);
		strcpy(filename, fullpath2);
		
		return 0;
	}

	if (strlen(p+1)+1 > 32)
	{
		/* filename too big for ISO9660 */
		return -1;
	}

	strcpy(filename, p+1);
	p[1] = 0;

	if (fullpath2[0] == '/')
		strcpy(path, fullpath2+1);
	else
		strcpy(path, fullpath2);

	UmdNormalizeName(filename);

	return 0;
}

//0x000037E4
int FindFileLBA(char *filename, int lba, int dirSize, int isDir, Iso9660DirectoryRecord *retRecord)
{
	Iso9660DirectoryRecord *record;
	char name[32];
	u8 *p;
	int oldDirLen = 0;
	int pos, res;
	int remaining = 0;

	pos = lba * SECTOR_SIZE;

	if (SizeToSectors(dirSize) <= 8)
	{
		res = IsofsReadSectors(lba, SizeToSectors(dirSize), sectors, NULL);
	}
	else
	{
		remaining = SizeToSectors(dirSize) - 8;
		res = IsofsReadSectors(lba, 8, sectors, NULL);
	}

	if (res < 0)
	{
		return res;
	}

	p = sectors;
	record = (Iso9660DirectoryRecord *)p;

	while (1)
	{
		if (record->len_dr == 0)
		{
			if (SECTOR_SIZE - (pos % SECTOR_SIZE) <= oldDirLen)
			{
				p += (SECTOR_SIZE - (pos % SECTOR_SIZE));
				pos += (SECTOR_SIZE - (pos % SECTOR_SIZE));				
				record = (Iso9660DirectoryRecord *)p;

				if (record->len_dr == 0)
				{
					return 0x80020001;
				}
			}else{
				return 0x80020002;
			}		
		}

		if (record->len_fi > 32)
		{
			return -1;
		}

		if (record->fi == 0)
		{
			if (strcmp(filename, ".") == 0)
			{
				memcpy(retRecord, record, sizeof(Iso9660DirectoryRecord));
				return record->lsbStart;
			}
		}

		else if(record->fi == 1)
		{
			if (strcmp(filename, "..") == 0)
			{
				memcpy(retRecord, record, sizeof(Iso9660DirectoryRecord));
				return record->lsbStart;
			}
		}

		else
		{
			memset(name, 0, 32);
			memcpy(name, &record->fi, record->len_fi);
			UmdNormalizeName(name);

			if (strcmp(name, filename) == 0)
			{
				if (isDir)
				{
					if (!(record->fileFlags & ISO9660_FILEFLAGS_DIR))
					{
						return 0x80020003;
					}
				}

				memcpy(retRecord, record, sizeof(Iso9660DirectoryRecord));
				return record->lsbStart;
			}
		}

		pos += record->len_dr;
		p += record->len_dr;
		oldDirLen = record->len_dr;
		record = (Iso9660DirectoryRecord *)p;
		
		if (remaining > 0)
		{
			int offset = (p - sectors);

			if ((offset + sizeof(Iso9660DirectoryRecord) + 0x60) >= (8*SECTOR_SIZE))
			{
				lba += (offset / SECTOR_SIZE);
				res = IsofsReadSectors(lba, 8, sectors, NULL);

				if (res < 0)
				{
					return res;
				}
				
				if (offset >= (8*SECTOR_SIZE))
				{
					remaining -= 8;
				}
				else
				{
					remaining -= 7;
				}

				p = sectors + (offset % SECTOR_SIZE);
				record = (Iso9660DirectoryRecord *)p;
			}
		}
	}

	return 0x80020004;
}

//0x00003C4C
int FindPathLBA(char *path, Iso9660DirectoryRecord *retRecord)
{
	char *p, *curpath;
	char curdir[32];
	int lba;
	int level = 0;

	lba = main_record.lsbStart;
	memcpy(retRecord, &main_record, sizeof(Iso9660DirectoryRecord));

	p = strchr(path, '/');
	curpath = path;

	while (p)
	{
		if (p-curpath+1 > 32)
		{
			return 0x80030001;
		}

		memset(curdir, 0, sizeof(curdir));
		strncpy(curdir, curpath, p-curpath);

		if (strcmp(curdir, ".") == 0)
		{

		}
		else if (strcmp(curdir, "..") == 0)
		{
			level--;
		}
		else
		{
			level++;
		}

		if (level > 8)
		{
			return 0x80030002;
		}
		
		lba = FindFileLBA(curdir, lba, retRecord->lsbDataLength, 1, retRecord);

		if (lba < 0)
			return lba;
		
		curpath = p+1;
		p = strchr(curpath, '/');		
	}
	
	return lba;
}

File* GetFile(char* fullpath){
	int lba;
	char path[256]="", filename[32]="";
	Iso9660DirectoryRecord record;

	lba = GetPathAndName(fullpath, path, filename);
	lba = FindPathLBA(path, &record);
	lba = FindFileLBA(filename, lba, record.lsbDataLength, 0, &record);

	File* file = (File*)malloc(sizeof(File));
	file->addr = lba * SECTOR_SIZE;
	file->size = record.lsbDataLength;
	strcpy(file->path, fullpath);
	strcpy(file->name, filename);

	if((u32)lba&0x80000000){
		return (File*)lba;
	}
	return file;
}
