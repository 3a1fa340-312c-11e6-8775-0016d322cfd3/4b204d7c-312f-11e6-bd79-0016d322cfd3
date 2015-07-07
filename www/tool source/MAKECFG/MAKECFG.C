#include <stdio.h>
#include <stdlib.h>

//#define MAX_WWW_SIZE					( 256 * 1024 )	// Roper's .WWW exceeds 180KB
#define MAX_WWW_SIZE					( 384 * 1024 )	// George modified on July 10, 2009
#define MAX_CHARS_PER_LINE				16


FILE				*OUTFile = NULL;
unsigned char		pHexText[MAX_CHARS_PER_LINE*6];
unsigned char		pComment[MAX_CHARS_PER_LINE+1];
int					nChars = 0;

void FlushData()
{
	static int 		nLines = 0;
	int				i;

	for( i = 0; i < nChars; i++ )
	{
		if( pComment[i] <= 0x1F || pComment[i] >= 0x7F )
			pComment[i] = '.';
		if( pComment[i] == '/' )
			pComment[i] = '.';
		if( pComment[i] == '\\' )
			pComment[i] = '.';
	}
	for( ; i < MAX_CHARS_PER_LINE; i++ )
		pComment[i] = ' ';
	pComment[i] = 0;

	fprintf( OUTFile, "/* %05X %s */ ", nLines++ * MAX_CHARS_PER_LINE, pComment );

	fprintf( OUTFile, pHexText );
	
	fprintf( OUTFile, "\r\n" );

	nChars = 0;
}

void ProduceData( unsigned char *pData, int nDataLen )
{
	int				nCount = 0;

	while( nCount < nDataLen )
	{
		if( nChars == 0 )
			memset( pHexText, 0, sizeof(pHexText) );

		sprintf( &pHexText[strlen(pHexText)], "0x%02X,", pData[nCount] );
		pComment[nChars] = pData[nCount];
		nChars++;
		nCount++;

		if( nChars == MAX_CHARS_PER_LINE )
			FlushData();
	}
}

void FillZero( int nLength )
{
	int				nCount = 0;

	while( nCount < nLength )
	{
		if( nChars == 0 )
			memset( pHexText, 0, sizeof(pHexText) );

		sprintf( &pHexText[strlen(pHexText)], "0x00," );
		pComment[nChars] = 0x00;
		nChars++;
		nCount++;

		if( nChars == MAX_CHARS_PER_LINE )
			FlushData();
	}

	if( nChars )
		FlushData();
}

void main( int nArgc, char *szArgv[] )
{
	FILE			*WWWFile = NULL;
	unsigned char	*pWWWMem = NULL;
	int				nWWWLen;

	if( nArgc == 2 || nArgc == 3 )
	{
		if( nArgc == 3 )
			OUTFile = fopen( szArgv[2], "wb" );
		else
			OUTFile = fopen( "ENDMARK.C", "wb" );
		if( OUTFile == NULL )
		{
			printf( "failed to open ENDMARK.C !" );
			goto Exit;
		}		

		WWWFile = fopen( szArgv[1], "rb" );
		if( WWWFile == NULL )
		{
			printf( "failed to open %s !", szArgv[1] );
			goto Exit;
		}

		pWWWMem = malloc( MAX_WWW_SIZE );
		if( pWWWMem == NULL )
		{
			printf( "Not enought memory !" );
			goto Exit;
		}
			
   		nWWWLen = fread( pWWWMem, 1, MAX_WWW_SIZE, WWWFile );
		if( nWWWLen <= 0 )
		{
			printf( "Failed to read %s !", szArgv[2] );
			goto Exit;
		}

		fprintf( OUTFile, "const char EndMark[]=\"{EndMark}\";\r\n" );

		fprintf( OUTFile, "const unsigned long EndMark1 = 0x00544F5A;\r\n" );
		
		fprintf( OUTFile, "const unsigned long MyDataSize[4] = { %d, %d, %d, 0 };\r\n", 
				MAX_WWW_SIZE, 0, nWWWLen );

		fprintf( OUTFile, "const unsigned char MyData[%d] = { \r\n", MAX_WWW_SIZE );

		
		ProduceData( pWWWMem, nWWWLen );

		FillZero( ( MAX_WWW_SIZE  ) - nWWWLen );

		fprintf( OUTFile, "};\r\n" );
	}
Exit:
	if( pWWWMem )
		free( pWWWMem );
	
	if( WWWFile )
		fclose( WWWFile );
	
	if( OUTFile )
		fclose( OUTFile );
}
