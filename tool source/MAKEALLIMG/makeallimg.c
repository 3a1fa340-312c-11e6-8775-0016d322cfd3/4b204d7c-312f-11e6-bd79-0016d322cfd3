#include <stdio.h>
#include <stdlib.h>

#define MAX_BIN_SIZE					( 1024 * 1024 )		// 1M
#define MAX_LOADER_SIZE					( 64 * 1024 )		// 64K
#define MAX_CODE1_SIZE					( 128 * 1024 )		// 128K
#define MAX_CODE2_SIZE					( 256 * 1024 )		// 256K

void main( int nArgc, char *szArgv[] )
{
	FILE			*BINFile = NULL;
	FILE			*LOADERFile = NULL;
	FILE			*CODE1File = NULL;
	FILE			*CODE2File = NULL;
	
	unsigned char	*pBINMem = NULL;
	unsigned char	*temp = NULL;
	int				nLOADERLen;
	int				nCODE1Len;
	int				nCODE2Len;

	if( nArgc == 5 )
	{

		LOADERFile = fopen( szArgv[1], "rb" );
		if( LOADERFile == NULL )
		{
			printf( "failed to open %s !", szArgv[1] );
			goto Exit;
		}
		
		CODE1File = fopen( szArgv[2], "rb" );
		if( CODE1File == NULL )
		{
			printf( "failed to open %s !", szArgv[2] );
			goto Exit;
		}
		
		CODE2File = fopen( szArgv[3], "rb" );
		if( CODE2File == NULL )
		{
			printf( "failed to open %s !", szArgv[3] );
			goto Exit;
		}

		BINFile = fopen( szArgv[4], "wb" );
		if( BINFile == NULL )
		{
			printf( "failed to open %s !", szArgv[4] );
			goto Exit;
		}		

		pBINMem = malloc( MAX_BIN_SIZE );
		if( pBINMem == NULL )
		{
			printf( "Not enought memory !" );
			goto Exit;
		}
		
		memset(pBINMem, 0xFF, MAX_BIN_SIZE);
		
		temp = pBINMem;  			
   		nLOADERLen = fread( temp, 1, MAX_BIN_SIZE, LOADERFile );
		if( nLOADERLen <= 0 )
		{
			printf( "Failed to read %s !", szArgv[1] );
			goto Exit;
		}
		if( nLOADERLen > MAX_LOADER_SIZE)
		{
			printf( " %s too big!", szArgv[1] );
			goto Exit;
		}
		
		temp = pBINMem + MAX_LOADER_SIZE;
		nCODE1Len = fread( temp , 1, MAX_BIN_SIZE - MAX_LOADER_SIZE, CODE1File );
		if( nCODE1Len <= 0 )
		{
			printf( "Failed to read %s !", szArgv[2] );
			goto Exit;
		}
		if( nCODE1Len > MAX_CODE1_SIZE)
		{
			printf( " %s too big!", szArgv[2] );
			goto Exit;
		}
		
		temp = pBINMem + MAX_LOADER_SIZE + MAX_CODE1_SIZE;
		nCODE2Len = fread( temp , 1, MAX_BIN_SIZE - (MAX_LOADER_SIZE + MAX_CODE1_SIZE) , CODE2File );
		if( nCODE1Len <= 0 )
		{
			printf( "Failed to read %s !", szArgv[3] );
			goto Exit;
		}
		if( nCODE1Len > MAX_CODE2_SIZE)
		{
			printf( " %s too big!", szArgv[3] );
			goto Exit;
		}
		
		fwrite( pBINMem, 1, MAX_BIN_SIZE, BINFile );

	}
	else
		printf( "Please input: makeallimg loader code1 code2 all" );
	
Exit:
	if( pBINMem )
		free( pBINMem );

	if( LOADERFile )
		fclose( LOADERFile );
		
	if( CODE1File )
		fclose( CODE1File );
	
	if( CODE2File )
		fclose( CODE2File );
	
	if( BINFile )
		fclose( BINFile );
}
