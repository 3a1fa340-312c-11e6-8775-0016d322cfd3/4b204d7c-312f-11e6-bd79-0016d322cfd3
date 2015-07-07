#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_CHARS_PER_LINE			1024

unsigned char		pszFileS[256]={0};
unsigned char		pszFileTmp[256]={0};
unsigned char		szLine[MAX_CHARS_PER_LINE];
unsigned char		szCopyright[MAX_CHARS_PER_LINE];
unsigned int		nMajorVersion;
unsigned int		nMinorVersion;
unsigned int		nPSModel;
unsigned int		nReleaseVersion;
unsigned int		nBuildVersion;
unsigned long		dwRDVersion;

int main(int nArgc, char *szArgv[])
{

	FILE		*SrcFile = NULL;
	FILE		*TmpFile = NULL;
	char		*pszValue;
	char 		szTemp1[30], szTemp2[30];
	time_t		ltime;
	struct  tm  *tm_ptr;
	int			nFound = 0;
	
	strcpy(pszFileS, "Target.def");

	SrcFile = fopen( pszFileS, "r+" );

	if( SrcFile == NULL )
	{
		printf( "failed to open %s !", pszFileS );
		goto Exit;
	}
	
	strcpy(pszFileTmp, "Temp.def");

	TmpFile = fopen( pszFileTmp, "w+" );
	if( TmpFile == NULL )
	{
		printf( "failed to open %s !", pszFileTmp );
		goto Exit;
	}	
	
	while( fgets( szLine, MAX_CHARS_PER_LINE, SrcFile ) )
	{
		if( nFound == 0 && strncmp( szLine, "PSMODELINDEX", strlen("PSMODELINDEX") ) == 0 )
			nFound = 1;
		if( nFound != 1 )
			fputs( szLine, TmpFile );
		if( strncmp( szLine, "MAKER_AND_CPU", strlen("MAKER_AND_CPU") ) == 0 )
		{
			pszValue = (char *)strstr( szLine, "=" );
			if( pszValue )
			{
				pszValue += 2;
				sscanf( pszValue, "%s", szTemp2 );
			}	
		}
		if( nFound == 1 )	
		{
			if( strncmp( szLine, "PSMODELINDEX", strlen("PSMODELINDEX") ) == 0 )
			{
				fputs( szLine, TmpFile );
				pszValue = (char *)strstr( szLine, "=" );
				if( pszValue )
				{
					pszValue += 2;
					sscanf( pszValue, "%d", &nPSModel );
				}	
			}
			else if( strncmp( szLine, "MajorVer", strlen("MajorVer") ) == 0 )
			{
				fputs( szLine, TmpFile );
				pszValue = (char *)strstr( szLine, "=" );
				if( pszValue )
				{
					pszValue += 2;
					sscanf( pszValue, "%d", &nMajorVersion );

				}	
			}
			else if( strncmp( szLine, "MinorVer", strlen("MinorVer") ) == 0 )
			{
				fputs( szLine, TmpFile );
				pszValue = (char *)strstr( szLine, "=" );
				if( pszValue )
				{
					pszValue += 2;
					sscanf( pszValue, "%d", &nMinorVersion );
				}	
			}
			else if( strncmp( szLine, "ReleaseVer", strlen("ReleaseVer") ) == 0 )
			{
				fputs( szLine, TmpFile );
				pszValue = (char *)strstr( szLine, "=" );
				if( pszValue )
				{
					pszValue += 2;
					sscanf( pszValue, "%d", &nReleaseVersion );
				}	
			}
			else if( strncmp( szLine, "RDVersion", strlen("RDVersion") ) == 0 )
			{
				pszValue = (char *)strstr( szLine, "=" );
				if( pszValue )
				{
					pszValue += 2;
					sscanf( pszValue, "%d", &dwRDVersion );
					dwRDVersion++;
					sprintf(pszValue,"%d",dwRDVersion);
					fputs( szLine, TmpFile );
					fputs( "\n", TmpFile );
				}	
			}
			else if( strncmp( szLine, "BuildVer", strlen("BuildVer") ) == 0 )
			{
				fputs( szLine, TmpFile );
				pszValue = (char *)strstr( szLine, "=" );
				if( pszValue )
				{
					pszValue += 2;
					sscanf( pszValue, "%d", &nBuildVersion );
				}	
			}
			else if( strncmp( szLine, "FirmwareString", strlen("FirmwareString") ) == 0 )
			{

				pszValue = (char *)strstr( szLine, "=" );
				if( pszValue )
				{
					pszValue += 2;
						
					time(&ltime);
					tm_ptr = localtime(&ltime);
					strftime( szTemp1, sizeof(szTemp1), "%Y/%m/%d %H:%M:%S", tm_ptr );
					
					sprintf( pszValue, "(%s-%d.%02d.%02d.%04d.%08d%c-%s)", 
						szTemp2, nMajorVersion, nMinorVersion, nPSModel, nBuildVersion, dwRDVersion, nReleaseVersion, szTemp1 );
					
					fputs( szLine, TmpFile );
					fputs( "\n", TmpFile );
				}
				nFound++;
			}
			
		}		
	}
	if( nFound > 1 )
	{
		if( SrcFile )
			fclose( SrcFile );
		SrcFile = NULL;
		if( TmpFile )
			fclose( TmpFile );
		TmpFile = NULL;

		remove( pszFileS );
		rename( pszFileTmp, pszFileS );
	}
	
Exit:
	if( SrcFile )
		fclose( SrcFile );
	if( TmpFile )
		fclose( TmpFile );	

}
