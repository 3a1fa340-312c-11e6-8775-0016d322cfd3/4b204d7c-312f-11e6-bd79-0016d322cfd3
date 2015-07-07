#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

#define MAX_CHARS_PER_LINE			1024

unsigned char		pszFileS[256];
unsigned char		pszFileTmp[256];
unsigned char		pszFileLog[256];
unsigned char		szLine[MAX_CHARS_PER_LINE];
unsigned char		szCopyright[MAX_CHARS_PER_LINE];
unsigned int		nMajorVersion;
unsigned int		nMinorVersion;
unsigned int		nPSModel;
unsigned int		nReleaseVersion;
unsigned int		nBuildVersion;
unsigned long		dwRDVersion;

void main(int nArgc, char *szArgv[])
{
	FILE		*SrcFile = NULL;
	FILE		*LogFile = NULL;
	FILE		*TmpFile = NULL;
	char		*pszValue;
	char 		szTemp1[20], szTemp2[20], szTemp3[30];
	time_t		ltime;
	int			nValue, nRead, nFound = 0, i;

	if( nArgc == 10 )
	{
		strcpy( pszFileS, szArgv[1] );
		
		strcpy( pszFileTmp, szArgv[2] );

		SrcFile = fopen( pszFileS, "r+" );

		if( SrcFile == NULL )
		{
			printf( "failed to open %s !", pszFileS );
			goto Exit;
		}		

		TmpFile = fopen( pszFileTmp, "w+" );
		if( TmpFile == NULL )
		{
			printf( "failed to open %s !", pszFileTmp );
			goto Exit;
		}

		nPSModel = atoi(szArgv[3]); //PS Model Index

		nMajorVersion = atoi(szArgv[4]);

    	nMinorVersion = atoi(szArgv[5]);

    	nReleaseVersion = atoi(szArgv[6]);

    	dwRDVersion = atoi(szArgv[7]);
    	dwRDVersion++;

    	nBuildVersion = atoi(szArgv[8]);
		
		strcpy( szTemp3, szArgv[9] );
		
		while( fgets( szLine, MAX_CHARS_PER_LINE, SrcFile ) )
		{
			if( nFound == 0 && strncmp( szLine, "MajorVersion", strlen("MajorVersion") ) == 0 )
				nFound = 1;
			if( nFound != 1 )
				fputs( szLine, TmpFile );
			if( nFound == 1 )
			{
				if( strncmp( szLine, "MajorVersion", strlen("MajorVersion") ) == 0 )
				{
					fputs( szLine, TmpFile );
					fgets( szLine, MAX_CHARS_PER_LINE, SrcFile );
					pszValue = strstr( szLine, "0x" );
					if( pszValue )
					{
						pszValue += 2;
						fprintf( TmpFile, "\t\tDCB\t\t0x%02X\t\t; Major Version\n", nMajorVersion );
					}	
				}
				else if( strncmp( szLine, "MinorVersion", strlen("MinorVersion") ) == 0 )
				{
					fputs( szLine, TmpFile );
					fgets( szLine, MAX_CHARS_PER_LINE, SrcFile );
					pszValue = strstr( szLine, "0x" );
					if( pszValue )
					{
						pszValue += 2;
						fprintf( TmpFile, "\t\tDCB\t\t0x%02X\t\t; Minor Version\n", nMinorVersion );
					}	
				}
				else if( strncmp( szLine, "PSModelNumber", strlen("PSModelNumber") ) == 0 )
				{
					fputs( szLine, TmpFile );
					fgets( szLine, MAX_CHARS_PER_LINE, SrcFile );
					pszValue = strstr( szLine, "0x" );
					if( pszValue )
					{
						pszValue += 2;
						fprintf( TmpFile, "\t\tDCB\t\t0x%02X\t\t; Model\n", nPSModel );
					}	
				}
				else if( strncmp( szLine, "ReleaseVersion", strlen("ReleaseVersion") ) == 0 )
				{
					fputs( szLine, TmpFile );
					fgets( szLine, MAX_CHARS_PER_LINE, SrcFile );
					pszValue = strstr( szLine, "0x" );
					if( pszValue )
					{
						pszValue += 2;
						fprintf( TmpFile, "\t\tDCB\t\t0x%02X\t\t; Release Version\n", nReleaseVersion );
					}	
				}
				else if( strncmp( szLine, "RDVersion", strlen("RDVersion") ) == 0 )
				{
					fputs( szLine, TmpFile );
					fgets( szLine, MAX_CHARS_PER_LINE, SrcFile );
					pszValue = strstr( szLine, "0x" );
					if( pszValue )
					{
						pszValue += 2;
						fprintf( TmpFile, "\t\tDCD\t\t0x%08X\t; RD Version\n", dwRDVersion );
					}	
				}
				else if( strncmp( szLine, "BuildVersion", strlen("BuildVersion") ) == 0 )
				{
					fputs( szLine, TmpFile );
					fgets( szLine, MAX_CHARS_PER_LINE, SrcFile );
					pszValue = strstr( szLine, "0x" );
					if( pszValue )
					{
						pszValue += 2;
						fprintf( TmpFile, "\t\tDCW\t\t0x%04X\t\t; Build Version\n", nBuildVersion );
					}	
				}
				else if( strncmp( szLine, "FirmwareString", strlen("FirmwareString") ) == 0 )
				{
					fputs( szLine, TmpFile );
					do
					{
						fgets( szLine, MAX_CHARS_PER_LINE, SrcFile );
					} while( strstr( szLine, "ALIGN" ) == 0 );
					//_strdate( szTemp1 );
					time(&ltime);
					strftime( szTemp1, sizeof(szTemp1), "%Y/%m/%d", localtime(&ltime) );
					_strtime( szTemp2 );

					sprintf( szCopyright, "%s-%X.%02X.%02d.%04X.%08d%c-%s %s", 
						szTemp3, nMajorVersion, nMinorVersion, nPSModel, nBuildVersion, dwRDVersion, nReleaseVersion, szTemp1, szTemp2 );

					fputs( "\t\t;", TmpFile );
					fputs( szCopyright, TmpFile );
					fputs( "\n", TmpFile );

					for( i = 0; i < 70; i++ )
					{
						if( i % 10 == 0 )
							fputs( "\t\tDCB\t\t", TmpFile );
						if( i < strlen(szCopyright) )
							sprintf( szTemp1, "0x%02X", szCopyright[i] );
						else
							sprintf( szTemp1, "0x%02X", 0 );
						fputs( szTemp1, TmpFile );
						if( i % 10 != 9 )
							fputs( ",", TmpFile );
						else
							fputs( "\n", TmpFile );
					}
					fputs( "\n", TmpFile );

					fputs( szLine, TmpFile );
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
	}
	else
	{
		printf( "    LOGFILE [source file1]  [source file2] [PS Model] [MajorVer] [MinorVer] [ReleaseVer] [RDVersion] [BuildVer] [MAKER_AND_CPU] ...\n");
	}
	
Exit:
	if( SrcFile )
		fclose( SrcFile );
	if( LogFile )
		fclose( LogFile );
	if( TmpFile )
		fclose( TmpFile );
}
