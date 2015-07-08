#ifndef _PFS_H
#define _PFS_H

#define PFS_SLASH '/'

struct e_pfsdirlist {
	uint32  StartFileList;
	uint32  NextDirList;
	uint8  DirName[1];
} __attribute__ ((aligned(1),packed));

typedef struct e_pfsdirlist PFS_DIR_LIST;

struct e_pfsfilelist {
	uint32  StartFileData;
	uint32  FileSize;
	uint32  NextFileList;
	uint8  FileName[1];
}__attribute__ ((aligned(1),packed));

typedef struct e_pfsfilelist PFS_FILE_LIST;

typedef struct {
	uint8 *pStartFileData;
	uint32 FileSize;
	uint32 CurFilePos;       //current file position
} PFS_FILE;

PFS_FILE *PFSopen(char *FullFileName);
WORD PFSread(BYTE *buffer,WORD size, PFS_FILE *pfile);
WORD PFSgets(BYTE *buffer, WORD size, PFS_FILE  *pfile);
void PFSclose(PFS_FILE *pfile);
extern PFS_DIR_LIST  *pDirList;   //9/20/99 added

#endif _PFS_H
