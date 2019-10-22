#ifndef CHESS_H
#define CHESS_H

#ifdef __cplusplus
extern "C" {
#endif

#define BOARD_SIZE  (64)

#define IV    0xFF
#define EP    0x00
#define BK    0x11
#define BQ    0x12
#define BR    0x13
#define BB    0x15
#define BN    0x17
#define BP    0x19
#define WK    0x21
#define WQ    0x22
#define WR    0x23
#define WB    0x25
#define WN    0x27
#define WP    0x29

struct chess;

enum chess_status
{
  CHESS_STATUS_NORMAL,
  CHESS_STATUS_WHITE_WIN,
  CHESS_STATUS_BLACK_WIN,
  CHESS_STATUS_DRAW,
  CHESS_STATUS_INVALID
};

enum chess_player
{
  CHESS_PLAYER_INVALID,
  CHESS_PLAYER_WHITE,
  CHESS_PLAYER_BLACK
};

// 尝试解决升变问题、悔棋问题、状态检查问题
// 王车易位时，经过的路径不能被对方攻击，否则易位无效

struct chess *chess_init( enum chess_player _player );
void chess_free( struct chess *_chess );
int  chess_replay( struct chess *_chess );
int  chess_play( struct chess *_chess,int _sx,int _sy,int _dx,int _dy );
int  chess_last_play( const struct chess *_chess, int *_x, int *_y );
int  chess_set_player( struct chess *_chess, enum chess_player _player );
int  chess_step( const struct chess *_chess );
int  chess_board( const struct chess *_chess, char _board[BOARD_SIZE] );
enum chess_player chess_curr_player( const struct chess *_chess );
enum chess_player chess_prev_player( const struct chess *_chess );
enum chess_status chess_curr_status( const struct chess *_chess );
int  chess_chess( const struct chess *_chess, int x, int y );

#ifdef __cplusplus
}
#endif

#endif

