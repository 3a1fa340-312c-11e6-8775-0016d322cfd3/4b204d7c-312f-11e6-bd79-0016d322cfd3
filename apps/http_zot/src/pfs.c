////////////////////////////////////////////////////////////////////////////
// Personal File System :
// (1) one support one subdirectory
// (2) Total size must less than 64k
// (3) path must use "/" not "\"
// (4) Only support 8.3 file format
////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cyg/kernel/kapi.h>
#include "pstarget.h"
#include "psglobal.h"
#include "pfs.h"

#ifdef HTTPD

extern int stricmp( char * dst,  char * src );

PFS_DIR_LIST  *pDirList;

static PFS_FILE_LIST *SearchDirName(BYTE *DirName);
static PFS_FILE *SearchFileName(PFS_FILE_LIST *pFileList, BYTE *FileName);

PFS_FILE *PFSopen(char *FullFileName)
{
	int8 *TmpSlash;
	BYTE RootDir[2];
	BYTE *DirName,*FileName;
    PFS_FILE_LIST *pFileList;
	PFS_FILE      *pFileNameStr;

	if(pDirList == NULL) return NULL;

	if((TmpSlash = strchr(FullFileName,PFS_SLASH)) == NULL) {
		RootDir[0] = '.';
		RootDir[1] = '\0';
		DirName = RootDir;
		FileName = FullFileName;
	}
	else {
		*TmpSlash = '\0';
		DirName = FullFileName;
		FileName = TmpSlash+1;
	}

	pFileNameStr = NULL;
	if((pFileList = SearchDirName(DirName)) != NULL) {
		pFileNameStr = SearchFileName(pFileList,FileName);
	}

	if(TmpSlash) *TmpSlash = PFS_SLASH;

	return pFileNameStr;
}

PFS_FILE_LIST *SearchDirName(BYTE *DirName)
{
	PFS_DIR_LIST *pTmpDirList;

	pTmpDirList = pDirList;

	while(pTmpDirList) {
		if(!stricmp(pTmpDirList->DirName,DirName)) {
			return (PFS_FILE_LIST *)((BYTE*)pDirList+pTmpDirList->StartFileList);
		}
		if(pTmpDirList->NextDirList)
			pTmpDirList = (PFS_DIR_LIST *)((BYTE*)pDirList+pTmpDirList->NextDirList);
		else pTmpDirList = NULL;
	}
	return NULL;
}

PFS_FILE *SearchFileName(PFS_FILE_LIST *pFileList, BYTE *FileName)
{
	int i,j;
	PFS_FILE_LIST *pTmpFileList;
	PFS_FILE *pPFSFile;

	pTmpFileList = pFileList;

	while(pTmpFileList) {
		if(!stricmp(pTmpFileList->FileName,FileName)) {
			if((pPFSFile = malloc(sizeof(PFS_FILE))) == NULL) break;
			pPFSFile->pStartFileData = ((BYTE*)pDirList+pTmpFileList->StartFileData);
			pPFSFile->FileSize = pTmpFileList->FileSize;
			pPFSFile->CurFilePos = 0;
			return pPFSFile;
		}
		if(pTmpFileList->NextFileList)
//			pTmpFileList = (PFS_FILE_LIST *)((BYTE*)pDirList+NGET32(&pTmpFileList->NextFileList));
			pTmpFileList = (PFS_FILE_LIST *)((BYTE*)pDirList + pTmpFileList->NextFileList);
		else pTmpFileList = NULL;
	}
	return NULL;
}

WORD PFSread(BYTE *buffer,WORD size, PFS_FILE *pfile)
{
	WORD ReadSize;

	ReadSize = min(size,(pfile->FileSize - pfile->CurFilePos));
	memcpy(buffer,(pfile->pStartFileData+pfile->CurFilePos),ReadSize);
	pfile->CurFilePos += ReadSize;

	return ReadSize;
}

WORD PFSgets(BYTE *buffer, WORD size, PFS_FILE  *pfile)
{
	WORD MaxSize,ReadSize;
	BYTE *pFilePos;

	MaxSize = min(size-1,(pfile->FileSize - pfile->CurFilePos));
	ReadSize = 0;
	pFilePos = pfile->pStartFileData+pfile->CurFilePos;

	while(ReadSize < MaxSize) {
		*(buffer++) = pFilePos[ReadSize];
		if(pFilePos[ReadSize++] == '\n') break;
	}
	pfile->CurFilePos += ReadSize;
	*buffer = '\0';

	return ReadSize;
}


void PFSclose(PFS_FILE *pfile)
{
	free(pfile);
}

#endif HTTPD
