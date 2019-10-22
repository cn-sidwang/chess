#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chess.h"

#define K    0x01
#define Q    0x02
#define R    0x03
#define B    0x05
#define N    0x07
#define P    0x09

#define EMP  EP
#define BRL  BR
#define BRR  (BR+1)
#define BBL  BB
#define BBR  (BB+1)
#define BNL  BN
#define BNR  (BN+1)
#define BP1  BP
#define BP2  (BP+1)
#define BP3  (BP+2)
#define BP4  (BP+3)
#define BP5  (BP+4)
#define BP6  (BP+5)
#define BP7  (BP+6)
#define BP8  (BP+7)

#define WRL  WR
#define WRR  (WR+1)
#define WBL  WB
#define WBR  (WB+1)
#define WNL  WN
#define WNR  (WN+1)
#define WP1  WP
#define WP2  (WP+1)
#define WP3  (WP+2)
#define WP4  (WP+3)
#define WP5  (WP+4)
#define WP6  (WP+5)
#define WP7  (WP+6)
#define WP8  (WP+7)

#define VALID_POS(x,y)    ((x)>=0&&(x)<8&&(y)>=0&&(y)<8)
#define SUB_ABS(n1,n2)    ((n1)>(n2)? (n1)-(n2) : (n2)-(n1))
#define IS_BLACK(ce)      ((ce)>=BK && (ce)<=BP8)
#define IS_WHITE(ce)      ((ce)>=WK && (ce)<=WP8)

struct chess
{
	char board[BOARD_SIZE];
	enum chess_player init_player;
	enum chess_player curr_player;
	enum chess_status curr_status;
	int  pass_pawn;
	int  status;
	int  last_x;
	int  last_y;
	int  step;
};

static const char board[BOARD_SIZE] = \
{
	WRR, WNR, WBR, WK,  WQ,  WBL, WNL, WRL,
	WP8, WP7, WP6, WP5, WP4, WP3, WP2, WP1,
	EMP, EMP, EMP, EMP, EMP, EMP, EMP, EMP,
	EMP, EMP, EMP, EMP, EMP, EMP, EMP, EMP,
	EMP, EMP, EMP, EMP, EMP, EMP, EMP, EMP,
	EMP, EMP, EMP, EMP, EMP, EMP, EMP, EMP,
	BP1, BP2, BP3, BP4, BP5, BP6, BP7, BP8,
	BRL, BNL, BBL, BK,  BQ,  BBR, BNR, BRR
};

static void _init_board( _board, _player )
	char _board[BOARD_SIZE];
	enum chess_player _player;
{
	if( _player == CHESS_PLAYER_BLACK )
	{
		memcpy( _board, board, sizeof(char)*BOARD_SIZE );
	}
	else
	{
		int index = 0;
		while( index < BOARD_SIZE )
		{
			_board[index] = board[BOARD_SIZE-1-index];
			++index;
		}
	}
}

static int _convert_chess( _c )
	int _c;
{
	switch( _c )
	{
		case BK:   return BK;
		case BQ:   return BQ;
		case BRL:
		case BRR:  return BR;
		case BBL:
		case BBR:  return BB;
		case BNL:
		case BNR:  return BN;
		case BP1:
		case BP2: 
		case BP3: 
		case BP4:
		case BP5:
		case BP6: 
		case BP7: 
		case BP8:  return BP;
		case WK:   return WK;
		case WQ:   return WQ;
		case WRL:
		case WRR:  return WR;
		case WBL:
		case WBR:  return WB;
		case WNL:
		case WNR:  return WN;
		case WP1: 
		case WP2: 
		case WP3:
		case WP4:
		case WP5: 
		case WP6: 
		case WP7:
		case WP8:  return WP;
	}
	return _c==EP? EP : IV;
}

static inline int _check_play_chess( _c, _player )
	int _c;
	enum chess_player _player;
{
	if( _c == EP || _player == CHESS_PLAYER_INVALID )
	{
		return -1;
	}

	return _player==CHESS_PLAYER_WHITE? 
		IS_WHITE(_c)? 0 : -1 : IS_BLACK(_c)? 0 : -1;
}

static inline int  _flags( _f, _i )
	int *_f;
	int  _i;
{
	return ( *_f & ( 1 << _i ) );
}

static inline void _flags_set( _f, _i )
	int *_f;
	int  _i;
{
	_flags( _f, _i ) == 0 && ( *_f = *_f ^ ( 1 << _i ) );
}

static inline void _flags_clear( _f, _i )
	int *_f;
	int  _i;
{
	_flags( _f, _i ) != 0 && ( *_f = *_f ^ ( 1 << _i ) );
}

static int _check_king_move( _sx, _sy, _dx, _dy, _chess )
	int _sx;
	int _sy;
	int _dx;
	int _dy;
	struct chess *_chess;
{
	int cx = SUB_ABS( _sx, _dx );
	int cy = SUB_ABS( _sy, _dy );
	int ce = _chess->board[ _sy*8+_sx ];
	if( ( 0xCF & _convert_chess(ce) )!= K )
	{
		return -1;
	}

	if( cx == 2 && cy == 0 )
	{
		int sx = _sx;
		if( _flags( &_chess->status, ce - BK )!=0 )
		{
			return -1;
		}

		ce = _chess->board[ _dy*8+((_sx - _dx)<0? 7:0) ];
		if( ( 0xCF & _convert_chess(ce) ) != R )
		{
			return -1;
		}

		if( _flags( &_chess->status, ce - BK )!=0 )
		{
			return -1;
		}

		sx += (_sx>_dx? -1 : 1);
		while( sx>0 && sx<7 )
		{
			if( _chess->board[ _sy*8+sx ] != EMP )
			{
				return -1;
			}
			sx += (_sx>_dx? -1 : 1);
		}
	}
	else if( cx>1 || cy>1 )
	{
		return -1;
	} 
	return 0;
}

static int _check_path( _sx, _sy, _dx, _dy, _chess )
	int _sx;
	int _sy;
	int _dx;
	int _dy;
	struct chess *_chess;
{
	int tx, ty, sx, sy, dx, dy;
	tx = _sx>_dx? -1 : _sx==_dx? 0 : 1;
	ty = _sy>_dy? -1 : _sy==_dy? 0 : 1;
	sx = _sx;  sy = _sy;  dx = _dx;  dy = _dy;
	for( sx+=tx, sy+=ty; sx!=dx || sy!=dy; sx+=tx, sy+=ty )
	{
		if( _chess->board[ sy*8+sx ] != EMP )
		{
			return -1;
		}
	}
	return 0;
}

static int _check_rook_move( _sx, _sy, _dx, _dy, _chess )
	int _sx;
	int _sy;
	int _dx;
	int _dy;
	struct chess *_chess;
{
	int ce = _chess->board[ _sy*8+_sx ];
	if( (0xCF & _convert_chess(ce) ) != R )
	{
		return -1;
	}

	if( _sx!=_dx && _sy!=_dy )
	{
		return -1;
	}

	if( _check_path( _sx, _sy, _dx, _dy, _chess )!=0 )
	{
		return -1;
	}

	return 0;
}

static int _check_bishop_move( _sx, _sy, _dx, _dy, _chess )
	int _sx;
	int _sy;
	int _dx;
	int _dy;
	struct chess *_chess;
{
	int ce = _chess->board[ _sy*8+_sx ];
	if( (0xCF & _convert_chess(ce) ) != B )
	{
		return -1;
	}

	if( _sx == _dx || _sy == _dy )
	{
		return -1;
	}

	if( SUB_ABS( _sx, _dx ) != SUB_ABS( _sy, _dy ) )
	{
		return -1;
	}

	if( _check_path( _sx, _sy, _dx, _dy, _chess )!=0 )
	{
		return -1;
	}

	return 0;
}

static int _check_queen_move( _sx, _sy, _dx, _dy, _chess )
	int _sx;
	int _sy;
	int _dx;
	int _dy;
	struct chess *_chess;
{
	int ce = _chess->board[ _sy*8+_sx ];

	if( (0xCF & _convert_chess(ce) ) != Q )
	{
		return -1;
	}

	if( _sx!=_dx && _sy!=_dy )
	{
		if( SUB_ABS(_sx, _dx) != SUB_ABS(_sy,_dy) )
		{
			return -1;
		}
	}

	if( _check_path( _sx, _sy, _dx, _dy, _chess ) != 0 )
	{
		return -1;
	}

	return 0;
}

static int _check_knight_move( _sx, _sy, _dx, _dy, _chess )
	int _sx;
	int _sy;
	int _dx;
	int _dy;
	struct chess *_chess;
{
	int tx, ty, ce;
	tx = SUB_ABS( _sx, _dx );
	ty = SUB_ABS( _sy, _dy );
	ce = _chess->board[ _sy*8+_sx ];

	if( (0xCF&_convert_chess(ce)) != N )
	{
		return -1;
	}

	if( _sx == _dx || _sy == _dy )
	{
		return -1;
	}

	if( tx > 2 || ty > 2 )
	{
		return -1;
	}

	if( tx==1? ty==2? 0 : 1 : tx==2? ty==1? 0 : 1 : 1 )
	{
		return -1;
	}

	return 0;
}

static int _check_pawn_move( _sx, _sy, _dx, _dy, _chess )
	int _sx;
	int _sy;
	int _dx;
	int _dy;
	struct chess *_chess;
{
	int ce, dr;
	ce = _chess->board[ _sy*8+_sx ];

	if( (0xCF & _convert_chess(ce)) != P )
	{
		return -1;
	}

	if( _sy == _dy )
	{
		return -1;
	}

	dr = _chess->init_player==CHESS_PLAYER_WHITE? 
		_chess->curr_player==CHESS_PLAYER_WHITE? -1 :  1
		:  _chess->curr_player==CHESS_PLAYER_WHITE?  1 : -1;

	if( ( (_sy - _dy) * dr ) >=0 )
	{
		return -1;
	}

	if( _sx!=_dx && _sy!=_dy )
	{
		if( SUB_ABS(_sx,_dx)!=1 || SUB_ABS(_sy,_dy)!=1 )
		{
			return -1;
		}

		if( _chess->board[ _dy*8+_dx ] == EMP )
		{
			ce = _chess->board[ _sy*8+_dx ];
			if( (_convert_chess(ce)&0xCF) != P || _chess->pass_pawn != ce )
			{
				return -1;
			}
		}
	}
	else
	{
		if( SUB_ABS( _sy, _dy )>2 )
		{
			return -1;
		}

		if( SUB_ABS( _sy, _dy )==2 )
		{
			if( _flags( &_chess->status, ce-BK )!=0 )
			{
				return -1;
			}
		}
		else
		{
			return _chess->board[ _dy*8+_dx ]==EMP? 0 : -1;
		}
	}
	return 0;
}

static int _check_rule( _sx, _sy, _dx, _dy, _chess )
	int _sx;
	int _sy;
	int _dx;
	int _dy;
	struct chess *_chess;
{
	if( _sx==_dx && _sy==_dy )
	{
		return -1;
	}

	if( _chess && VALID_POS(_sx, _sy) && VALID_POS(_dx, _dy) )
	{   
		int _sc = _chess->board[ _sy*8+_sx ];
		int _dc = _chess->board[ _dy*8+_dx ];

		if( _chess->curr_status != CHESS_STATUS_NORMAL )
		{
			return -1;
		}

		if( _check_play_chess( _sc, _chess->curr_player )!=0 )
		{
			return -1;
		}

		if( _dc != EMP )
		{
			if( _check_play_chess( _dc, _chess->curr_player )==0 )
			{
				return -1;
			}
		}

		switch( _convert_chess(_sc) & 0xCF )
		{
			case K: return _check_king_move(_sx, _sy, _dx, _dy, _chess);
			case Q: return _check_queen_move(_sx, _sy, _dx, _dy, _chess);
			case R: return _check_rook_move(_sx, _sy, _dx, _dy, _chess);
			case B: return _check_bishop_move(_sx, _sy, _dx, _dy, _chess);
			case N: return _check_knight_move(_sx, _sy, _dx, _dy, _chess);
			case P: return _check_pawn_move(_sx, _sy, _dx, _dy, _chess);
		}
	}
	return -1;
}

static void _play_chess_king( _sx, _sy, _dx, _dy, _board )
	int _sx;
	int _sy;
	int _dx;
	int _dy;
	char _board[BOARD_SIZE];
{
	_board[ _dy*8+_dx ] = _board[ _sy*8+_sx ];
	_board[ _sy*8+_sx ] = EMP;
	if( SUB_ABS( _sx, _dx )==2 && SUB_ABS( _sy, _dy )==0 )
	{
		int rsx = _sx>_dx? 0 : 7;
		int rdx = rsx==0? _dx+1 : _dx-1;
		_board[ _dy*8+rdx ] = _board[ _sy*8+rsx ];
		_board[ _sy*8+rsx ] = EMP;
	}
}

static void _play_chess_queen( _sx, _sy, _dx, _dy, _board )
	int _sx;
	int _sy;
	int _dx;
	int _dy;
	char _board[BOARD_SIZE];
{
	_board[ _dy*8+_dx ] = _board[ _sy*8+_sx ];
	_board[ _sy*8+_sx ] = EMP;
}

static void _play_chess_rook( _sx, _sy, _dx, _dy, _board )
	int _sx;
	int _sy;
	int _dx;
	int _dy;
	char _board[BOARD_SIZE];
{
	_board[ _dy*8+_dx ] = _board[ _sy*8+_sx ];
	_board[ _sy*8+_sx ] = EMP;
}

static void _play_chess_bishop( _sx, _sy, _dx, _dy, _board )
	int _sx;
	int _sy;
	int _dx;
	int _dy;
	char _board[BOARD_SIZE];
{
	_board[ _dy*8+_dx ] = _board[ _sy*8+_sx ];
	_board[ _sy*8+_sx ] = EMP;
}

static void _play_chess_knight( _sx, _sy, _dx, _dy, _board )
	int _sx;
	int _sy;
	int _dx;
	int _dy;
	char _board[BOARD_SIZE];
{
	_board[ _dy*8+_dx ] = _board[ _sy*8+_sx ];
	_board[ _sy*8+_sx ] = EMP;
}

static void _play_chess_pawn( _sx, _sy, _dx, _dy, _board )
	int _sx;
	int _sy;
	int _dx;
	int _dy;
	char _board[BOARD_SIZE];
{
	int se, de;
	se = _board[ _sy*8+_sx ];
	de = _board[ _dy*8+_dx ];
	_board[ _dy*8+_dx ] = _board[ _sy*8+_sx ];
	_board[ _sy*8+_sx ] = EMP;
	if( de==EMP && SUB_ABS(_sx,_dx)==1 && SUB_ABS(_sy,_dy)==1 )
	{
		_board[ _sy*8+_dx ] = EMP;
	}
}

static void _play_chess( _sx, _sy, _dx, _dy, _chess, _board )
	int _sx;
	int _sy;
	int _dx;
	int _dy;
	struct chess *_chess;
	char _board[BOARD_SIZE];
{
	int ce = _board[ _sy*8+_sx ];
	switch( 0xCF & _convert_chess(ce) )
	{
		case K: _play_chess_king(_sx,_sy,_dx,_dy,_board ); break;
		case Q: _play_chess_queen(_sx,_sy,_dx,_dy,_board); break;
		case R: _play_chess_rook(_sx,_sy, _dx,_dy,_board); break;
		case B: _play_chess_bishop(_sx,_sy,_dx,_dy,_board);break;
		case N: _play_chess_knight(_sx,_sy,_dx,_dy,_board);break;
		case P: _play_chess_pawn(_sx,_sy,_dx,_dy,_board);  break;
	}
}

static int _play_and_check_king( _sx, _sy, _dx, _dy, _chess )
	int _sx;
	int _sy;
	int _dx;
	int _dy;
	struct chess *_chess;
{
	int  kx = -1, ky = -1, ke, ce;
	char tmp_board[BOARD_SIZE] = {0};
	memcpy( tmp_board, _chess->board, BOARD_SIZE * sizeof(char) );
	_play_chess( _sx, _sy, _dx, _dy, _chess, tmp_board );

	ce = _chess->board[ _sy*8+_sx ];
	ke = _chess->curr_player==CHESS_PLAYER_WHITE? WK : BK;
	for( int i=0; i<8; i++ )
	{
		for( int j=0; j<8; j++ )
		{
			if( tmp_board[ i*8+j ] == ke )
			{
				kx = j;
				ky = i;
				break;
			}
		}
	}

	if( kx==-1 || ky==-1 )
	{
		return -1;
	}

	for( int i=0; i<8; i++ )
	{
		for( int j=0; j<8; j++ )
		{
			int ce = tmp_board[ j*8+i ];
			if( ce == EMP )
			{
				continue;
			}

			if( _check_play_chess(ce, _chess->curr_player)!=0 )
			{
				int ret = -1;
				switch( 0xCF & _convert_chess(ce) )
				{
					case K:
						ret = _check_king_move( j, i, kx, ky, _chess );
						break;
					case Q:
						ret = _check_queen_move( j, i, kx, ky, _chess );
						break;
					case R:
						ret = _check_rook_move( j, i, kx, ky, _chess );
						break;
					case B:
						ret = _check_bishop_move( j, i, kx, ky, _chess );
						break;
					case N:
						ret = _check_knight_move( j, i, kx, ky, _chess );
						break;
					case P:
						ret = _check_pawn_move( j, i, kx, ky, _chess );
						break;
				}
				if( ret == 0 )
				{
					return -1;
				}
			}
		}
	}

	_flags_set( &_chess->status, ce-BK );
	if( (0xCF & _convert_chess(ce)) == P )
	{
		if( SUB_ABS(_sy,_dy)==2 )
			_chess->pass_pawn = ce;
	}
	else
	{
		_chess->pass_pawn = EMP;
	}
	memcpy( _chess->board, tmp_board, BOARD_SIZE * sizeof(char) );
	return 0;
}

static void _check_status( _chess )
	struct chess *_chess;
{

}

static int _do_play( _sx, _sy, _dx, _dy, _chess )
	int _sx;
	int _sy;
	int _dx;
	int _dy;
	struct chess *_chess;
{
	int ce;
	if( _check_rule( _sx, _sy, _dx, _dy, _chess )!=0 )
	{
		return -1;
	}

	if( _play_and_check_king( _sx, _sy, _dx, _dy, _chess )!=0 )
	{
		return -1;
	}

	_chess->curr_player = _chess->curr_player==CHESS_PLAYER_WHITE?
		CHESS_PLAYER_BLACK : CHESS_PLAYER_WHITE;
	_check_status( _chess );
	return 0;
}

struct chess *chess_init( _player )
	enum chess_player _player;
{
	struct chess *_chess = NULL;
	_chess = (struct chess*)calloc(sizeof(struct chess),1);
	_chess->init_player = _player==CHESS_PLAYER_INVALID?
		CHESS_PLAYER_BLACK : _player;
	_chess->curr_player = CHESS_PLAYER_BLACK;
	_chess->curr_status = CHESS_STATUS_NORMAL;
	_chess->pass_pawn = 0;
	_chess->last_x = -1;
	_chess->last_y = -1;
	_chess->status = 0;
	_chess->step = 0;
	_init_board( _chess->board, _chess->init_player );
	return _chess;
}

void chess_free( _chess )
	struct chess *_chess;
{
	if( _chess )
	{
		free( _chess );
	}
}

int chess_replay( _chess )
	struct chess *_chess;
{
	if( _chess )
	{
		_init_board( _chess->board, _chess->init_player );
		_chess->curr_status = CHESS_STATUS_NORMAL;
		_chess->curr_player = CHESS_PLAYER_BLACK;
		_chess->pass_pawn = 0;
		_chess->last_x = -1;
		_chess->last_y = -1;
		_chess->status = 0;
		_chess->step = 0;
	}
}

int  chess_play( _chess, _sx, _sy, _dx, _dy )
	struct chess *_chess;
	int _sx;
	int _sy;
	int _dx;
	int _dy;
{
	return _do_play( _sx, _sy, _dx, _dy, _chess );
}

int  chess_last_play( _chess, _x, _y )
	const struct chess *_chess;
	int *_x;
	int *_y;
{
	if( _chess && _x && _y )
	{
		*_x = _chess->last_x;
		*_y = _chess->last_y;
		return 0;
	}
	return -1;
}

int  chess_set_player( _chess, _player )
	struct chess *_chess;
	enum chess_player _player;
{
	if( _chess && _player!=CHESS_PLAYER_INVALID )
	{
		_chess->init_player = _player;
		chess_replay( _chess );
		return 0;
	}
	return -1;
}

int  chess_step( _chess )
	const struct chess *_chess;
{
	return _chess? _chess->step : -1;
}

int chess_board( _chess, _board )
	const struct chess *_chess;
	char _board[BOARD_SIZE];
{
	if( _chess && _board )
	{
		for( int i=0; i<BOARD_SIZE; i++ )
		{
			_board[i] = _convert_chess( _chess->board[i] );
		}
		return 0;
	}
	return -1;
}

enum chess_player chess_curr_player( _chess )
	const struct chess *_chess;
{
	return _chess? _chess->curr_player : CHESS_PLAYER_INVALID;
}

enum chess_player chess_prev_player( _chess )
	const struct chess *_chess;
{
	if( _chess )
	{
		return _chess->curr_player==CHESS_PLAYER_WHITE?
			CHESS_PLAYER_BLACK : CHESS_PLAYER_WHITE;
	}
	return  CHESS_PLAYER_INVALID;
}

enum chess_status chess_curr_status( _chess )
	const struct chess *_chess;
{
	return _chess? _chess->curr_status : CHESS_STATUS_INVALID;
}

int  chess_chess( _chess, x, y )
	const struct chess *_chess;
	int x;
	int y;
{
	if( _chess && VALID_POS(x,y) )
	{
		return _convert_chess( _chess->board[y*8+x] );
	}
	return IV;
}

