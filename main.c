#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chess.h"

static int  running = 1;

void _show_board( _board )
    char _board[BOARD_SIZE];
{
    printf("\n   ");
    for( int i=0; i<8; i++ )
    {
	printf("%d  ", i );
    }
    printf("\n");
    for( int i=0; i<8; i++ )
    {
       printf("%d ", i );
       for( int j=0; j<8; j++ )
       {
          switch( _board[i*8+j] )
	  {
	  case BK: printf("BK "); break;
	  case BQ: printf("BQ "); break;
	  case BR: printf("BR "); break;
	  case BB: printf("BB "); break;
	  case BN: printf("BN "); break;
	  case BP: printf("BP "); break;
	  case WK: printf("WK "); break;
	  case WQ: printf("WQ "); break;
	  case WR: printf("WR "); break;
	  case WB: printf("WB "); break;
	  case WN: printf("WN "); break;
	  case WP: printf("WP "); break;
          default: printf("   ");
	  }
       }
       printf("\n");
    }
    printf("\n");
}

int main( argc, argv )
    int argc;
    char **argv;
{
    FILE *fp = NULL;
    char   line[256] = {0};
    struct chess *chess = NULL;
    char   board[BOARD_SIZE];
    int    sx, sy, dx, dy;

    chess = chess_init( CHESS_PLAYER_WHITE );
    if( chess==NULL )
    {
        printf("ERROR: chess_init failed!\n");
	return -1;
    }

    fp = fopen( "order.list", "r" );
    if( fp==NULL )
    {
        printf("ERROR: missing order.list\n");
	return -1;
    }
   
    while( !feof(fp) )
    {
        chess_board( chess, board );
	_show_board( board );
	do{
	   memset( line, 0, 256 );
	   fgets( line, 256, fp );
	   if( strcmp(line, "\n")!=0 )
	   {
		break;
	   }
	}while(!feof(fp));

	if( strcmp( line, "END" )==0 )
	{
           break;
	}

	fscanf( fp, "%d,%d", &sx, &sy );
	fscanf( fp, "%d,%d", &dx, &dy );
	printf("TEST: %s   ", line );
        if( chess_play( chess, sx, sy, dx, dy )==-1 )
	{
	    printf("[FAILED}\n");
	    break;
	}
	else
	{
	    printf("[SUCCESSED]\n");
	}
    }

    fclose(fp);
    chess_free( chess ); 
    return 0;
}

