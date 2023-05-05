#include "halma.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define FOURP_PIECES 13
#define TWOP_PIECES 19

#define EXIT_MALLOC_ERROR 39

#define check_malloc(ptr) \
  if (ptr == NULL) exit(EXIT_MALLOC_ERROR);

#define bad_file_break(ptr) \
  {                         \
    free(ptr);              \
    return NULL;            \
  }

// this allocates the exact number of bytes a save file is + 1
#define SAVE_BUFF_SIZE ((HALMA_SQUARE_ROOT * HALMA_SQUARE_ROOT * 2) + 7)

struct halma_board* halma_load_game(const char* filename) {
  FILE* savegame = fopen(filename, "rb");
  if (savegame == NULL) return NULL;
  char save_buffer[SAVE_BUFF_SIZE];
  struct halma_board* board = malloc(sizeof(struct halma_board));
  check_malloc(board);

  fgets(save_buffer, SAVE_BUFF_SIZE, savegame);
  fclose(savegame);
  // the whole file should now be in memory, these save games aren't that big.

  // the file is just a binary lump of the data, we validate the first byte to
  // see if its what we are expecting and then just dump the rest where it needs
  // to go

  if (save_buffer[0] != HALMA_SQUARE_ROOT) bad_file_break(board);
  if (save_buffer[1] != 2 && save_buffer[1] != 4) bad_file_break(board);

  board->players = save_buffer[1];
  board->player_pieces = save_buffer[2];

  // set char* to the beginning of the board in our array
  // row/col and endianness should be fine as this should all be single bytes
  char* save_board = &save_buffer[3];
  for (int i = 0; i < HALMA_SQUARE_ROOT; i++)
    for (int o = 0; o < HALMA_SQUARE_ROOT; o++)
      board->grid[i][o] = save_board[(i * HALMA_SQUARE_ROOT) + o];

  save_board += (HALMA_SQUARE_ROOT * HALMA_SQUARE_ROOT);
  for (int i = 0; i < HALMA_SQUARE_ROOT; i++)
    for (int o = 0; o < HALMA_SQUARE_ROOT; o++)
      board->victory_mask[i][o] = save_board[(i * HALMA_SQUARE_ROOT) + o];

  save_board += (HALMA_SQUARE_ROOT * HALMA_SQUARE_ROOT);
  short turns_divisor = save_board[0];  // just going to take it from the file,
                                        // in case its different
  board->turns = save_board[1] * turns_divisor;
  board->turns += save_board[2];

  return board;
}

int halma_save_game(const char* filename, struct halma_board* board) {
  FILE* savegame = fopen(filename, "w+b");
  if (savegame == NULL) return 1;
  putc(HALMA_SQUARE_ROOT, savegame);
  putc(board->players, savegame);
  putc(board->player_pieces, savegame);

  for (int i = 0; i < HALMA_SQUARE_ROOT; i++)
    for (int o = 0; o < HALMA_SQUARE_ROOT; o++)
      putc(board->grid[i][o], savegame);
  for (int i = 0; i < HALMA_SQUARE_ROOT; i++)
    for (int o = 0; o < HALMA_SQUARE_ROOT; o++)
      putc(board->victory_mask[i][o], savegame);

  // number of turns goes at the end, because its the only value greater than a
  // single byte we need to handle
  // this weird bit of math accounts for endian nasties
  putc(SCHAR_MAX, savegame);
  putc(board->turns / SCHAR_MAX, savegame);
  putc(board->turns % SCHAR_MAX, savegame);

  fclose(savegame);

  return 0;
}

dimension_T halma_validate_piece_selection(struct halma_board* board,
                                           dimension_T y_index,
                                           dimension_T x_index,
                                           struct halma_moves* moves,
                                           enum halma_piece turn) {
  // there isn't one of our pieces here, return 1
  if (board->grid[y_index][x_index] != turn) return -1;

  // locate this piece in the move table
  dimension_T i;
  for (i = 0; i < board->player_pieces; i++) {
    if (moves[i].origin_y == y_index && moves[i].origin_x == x_index) break;
  }

  // there was no move table located for this piece, somethings gone wrong,
  // return -2
  if (i == board->player_pieces) return -2;

  // this piece can't move anywhere, return 3
  if (!anybit(moves[i].targets)) return -3;

  // selection is good, return the move index for this piece
  return i;
}

bool halma_validate_target_selection(struct halma_moves* move,
                                     dimension_T y_index, dimension_T x_index) {
  if (getbit(move->targets, y_index, x_index)) return true;
  return false;
}

dimension_T halma_accept_move(struct halma_board* board,
                              struct halma_moves* move, dimension_T y_index,
                              dimension_T x_index) {
  if(!halma_validate_target_selection(move, y_index, x_index)) return -1;
  board->grid[y_index][x_index] = board->grid[move->origin_y][move->origin_x];
  board->grid[move->origin_y][move->origin_x] = EMPTY;
  if(board->turns == SHRT_MAX) return -2;
  board->turns++;
  return 0;
}

bool halma_check_victory(struct halma_board* board, enum halma_piece turn) {
  dimension_T player_pieces = 0;
  dimension_T nonplayer_pieces = 0;
  for (dimension_T i = 0; i < HALMA_SQUARE_ROOT; i++)
    for (dimension_T o = 0; o < HALMA_SQUARE_ROOT; o++)
      if (board->victory_mask[i][o] == turn) {
        if (board->grid[i][o] == EMPTY) return false;
        if (board->grid[i][o] == turn)
          player_pieces++;
        else
          nonplayer_pieces++;
      }
  if (player_pieces > 0 &&
      (player_pieces + nonplayer_pieces) == board->player_pieces)
    return true;
  return false;
}

bool halma_any_possible_moves(struct halma_board* board, struct halma_moves* moves){
  for(dimension_T i = 0; i < board->player_pieces; i++)
    if(anybit(moves[i].targets))
      return true;
  board->turns++;
  return false;
}

enum halma_piece halma_check_victory_all(struct halma_board* board) {
  if (halma_check_victory(board, RED)) return RED;
  if (halma_check_victory(board, YELLOW)) return YELLOW;
  if (board->players != 2) {
    if (halma_check_victory(board, GREEN)) return GREEN;
    if (halma_check_victory(board, BLUE)) return BLUE;
  }
  return EMPTY;
}

enum halma_piece halma_whos_turn(struct halma_board* board){
  return (board->turns % board->players) + 1;
}

/**
 * @brief Gets at max a 3x3 section of coordiantes centered on orig_y and orig_x
 * Will check the bounds and truncate if needed.
 */
#define halma_get_3x3_bounds(top_y, top_x, bot_y, bot_x, orig_y, orig_x) \
  top_y = orig_y - 1;                                                    \
  if (top_y < 0) top_y = 0;                                              \
  top_x = orig_x - 1;                                                    \
  if (top_x < 0) top_x = 0;                                              \
  bot_y = orig_y + 2;                                                    \
  if (bot_y >= HALMA_SQUARE_ROOT) bot_y = HALMA_SQUARE_ROOT;             \
  bot_x = orig_x + 2;                                                    \
  if (bot_x >= HALMA_SQUARE_ROOT) bot_x = HALMA_SQUARE_ROOT;

void halma_search_jump(struct halma_board* board, struct halma_moves* move,
                       dimension_T piece_y, dimension_T piece_x,
                       dimension_T origin_y, dimension_T origin_x) {
  dimension_T difference_y = piece_y + (piece_y - origin_y);
  dimension_T difference_x = piece_x + (piece_x - origin_x);

  // of the edge of the board, no more searches, I don't actually know if all of
  // these are possible to encounter
  if (difference_y < 0) return;
  if (difference_y >= HALMA_SQUARE_ROOT) return;
  if (difference_x < 0) return;
  if (difference_x >= HALMA_SQUARE_ROOT) return;

  if (board->grid[difference_y][difference_x] == EMPTY) {
    // mark this tile as movalbe to, if this tile
    // has already been marked as a good move, return.
    if (getsetbit(move->targets, difference_y, difference_x)) return;

    // search the area around this tile for other jumpable pieces
    dimension_T top_y, top_x, bot_y, bot_x;
    halma_get_3x3_bounds(top_y, top_x, bot_y, bot_x, difference_y,
                         difference_x);
    for (int i = top_y; i < bot_y; i++)
      for (int o = top_x; o < bot_x; o++)
        if (board->grid[i][o] != EMPTY)
          halma_search_jump(board, move, i, o, difference_y, difference_x);
  }
}

void halma_search_immediate(struct halma_board* board,
                            struct halma_moves* move) {
  dimension_T top_y, top_x, bot_y, bot_x;
  halma_get_3x3_bounds(top_y, top_x, bot_y, bot_x, move->origin_y,
                       move->origin_x);
  for (int i = top_y; i < bot_y; i++)
    for (int o = top_x; o < bot_x; o++)
      // immediate movements
      if (board->grid[i][o] == EMPTY) setbit(move->targets, i, o);
      // jump movements, only allowed to jump if we are NOT in the victory
      // location
      else
        halma_search_jump(board, move, i, o, move->origin_y, move->origin_x);
}

void halma_cull_victory_moves(struct halma_board* board,
                              struct halma_moves* move) {
  if (!anybit(move->targets))
    return;  // no valid moves, we don't need to check anything

  enum halma_piece turn = board->grid[move->origin_y][move->origin_x];
  // part of my goal when making this game was to never have to hammer through
  // an entire bitmask for anything, after some experimentation this was the
  // best I could do. Might be something to revisit down the line.
  for (dimension_T i = 0; i < HALMA_SQUARE_ROOT; i++)
    for (dimension_T o = 0; o < HALMA_SQUARE_ROOT; o++)
      if (board->victory_mask[i][o] != turn)
        if (getbit(move->targets, i, o)) togglebit(move->targets, i, o);
}

struct halma_moves* halma_gather_valid_moves(struct halma_board* board,
                                             enum halma_piece set) {
  struct halma_moves* moves_table =
      malloc(sizeof(struct halma_moves) * board->player_pieces);
  check_malloc(moves_table);
  dimension_T pieces_count = 0;
  for (dimension_T i = 0; i < HALMA_SQUARE_ROOT; i++)
    for (dimension_T o = 0; o < HALMA_SQUARE_ROOT; o++)
      if (board->grid[i][o] == set) {
        moves_table[pieces_count].targets =
            create_bitmask(HALMA_SQUARE_ROOT, HALMA_SQUARE_ROOT);
        moves_table[pieces_count].origin_y = i;
        moves_table[pieces_count].origin_x = o;

        halma_search_immediate(board, &moves_table[pieces_count]);

        // if we are inside of the victory area make sure we can't move outside
        // of it intermediate jumps outside are fine but if it started in the
        // victory area it needs to end in the victory area
        if ((board->grid[i][o] == board->victory_mask[i][o]))
          halma_cull_victory_moves(board, &moves_table[pieces_count]);

        pieces_count++;
      }

  return moves_table;
}

void halma_gen_victory_mask(struct halma_board* board) {
  // loop through the board swapping each color for whats on the opposite corner
  for (dimension_T i = 0; i < HALMA_SQUARE_ROOT; i++)
    for (dimension_T o = 0; o < HALMA_SQUARE_ROOT; o++)
      switch (board->grid[i][o]) {
        case RED:
          board->victory_mask[i][o] = YELLOW;
          break;
        case GREEN:
          board->victory_mask[i][o] = BLUE;
          break;
        case BLUE:
          board->victory_mask[i][o] = GREEN;
          break;
        case YELLOW:
          board->victory_mask[i][o] = RED;
          break;
        default:
          board->victory_mask[i][o] = EMPTY;
          break;
      }
}

struct halma_board* halma_init_board_4p() {
  struct halma_board* board = malloc(sizeof(struct halma_board));
  check_malloc(board);
  // set turns to zero
  board->turns = 0;

  // set number of players
  board->players = 4;

  // set number of pieces
  board->player_pieces = FOURP_PIECES;

  // initialize board to EMPTY
  for (dimension_T i = 0; i < HALMA_SQUARE_ROOT; i++)
    for (dimension_T o = 0; o < HALMA_SQUARE_ROOT; o++)
      board->grid[i][o] = EMPTY;

  // initialize RED pieces
  for (dimension_T i = 0; i < (HALMA_SQUARE_ROOT / 4); i++)
    for (dimension_T o = (HALMA_SQUARE_ROOT / 4) - i; o >= 0; o--)
      board->grid[i][o] = RED;
  // algorithm generates one extra tile on row 0
  // alogrithm would generate extra tile on other end of pieces
  // if the condition was i < (SQUARE_ROOT / 4) + 1, we skip the last row
  // because it is all extra. A similar thing happens to all corners.
  board->grid[0][(HALMA_SQUARE_ROOT / 4)] = EMPTY;

  // initialize GREEN pieces
  for (dimension_T i = 0; i < (HALMA_SQUARE_ROOT / 4); i++)
    for (dimension_T o = HALMA_SQUARE_ROOT - 1;
         o >= HALMA_SQUARE_ROOT - (HALMA_SQUARE_ROOT / 4) + i - 1; o--)
      board->grid[i][o] = GREEN;
  // remove extra piece at ends
  board->grid[0][HALMA_SQUARE_ROOT - (HALMA_SQUARE_ROOT / 4) - 1] = EMPTY;

  // initialize BLUE pieces
  for (dimension_T i = HALMA_SQUARE_ROOT - 1;
       i >= HALMA_SQUARE_ROOT - (HALMA_SQUARE_ROOT / 4); i--)
    for (dimension_T o = 0;
         o < (HALMA_SQUARE_ROOT / 4) - (HALMA_SQUARE_ROOT - i) + 2; o++)
      board->grid[i][o] = BLUE;
  // remove extra piece at end
  board->grid[HALMA_SQUARE_ROOT - 1][HALMA_SQUARE_ROOT / 4] = EMPTY;

  // intialize YELLOW pieces
  for (dimension_T i = HALMA_SQUARE_ROOT - 1;
       i >= HALMA_SQUARE_ROOT - (HALMA_SQUARE_ROOT / 4); i--)
    for (dimension_T o = HALMA_SQUARE_ROOT - 1;
         o >= HALMA_SQUARE_ROOT - (HALMA_SQUARE_ROOT / 4) - 1 +
                  (HALMA_SQUARE_ROOT - i - 1);
         o--)
      board->grid[i][o] = YELLOW;
  // remove extra pieces at end
  board->grid[HALMA_SQUARE_ROOT - 1]
             [HALMA_SQUARE_ROOT - 1 - (HALMA_SQUARE_ROOT / 4)] = EMPTY;

  // initialize the victory mask
  halma_gen_victory_mask(board);

  return board;
}

struct halma_board* halma_init_board_2p() {
  struct halma_board* board = malloc(sizeof(struct halma_board));
  check_malloc(board);
  // set turns to zero
  board->turns = 0;

  // set number of players
  board->players = 2;

  // set number of pieces
  board->player_pieces = TWOP_PIECES;

  // initialize board to EMPTY
  for (dimension_T i = 0; i < HALMA_SQUARE_ROOT; i++)
    for (dimension_T o = 0; o < HALMA_SQUARE_ROOT; o++)
      board->grid[i][o] = EMPTY;

  // initialize RED pieces
  for (dimension_T i = 0; i < (HALMA_SQUARE_ROOT / 3); i++)
    for (dimension_T o = (HALMA_SQUARE_ROOT / 3) - i; o >= 0; o--)
      board->grid[i][o] = RED;
  // algorithm generates one extra tile on row 0
  // alogrithm would generate extra tile on other end of pieces
  // if the condition was i < (SQUARE_ROOT / 4) + 1, we skip the last row
  // because it is all extra. A similar thing happens to all corners.
  board->grid[0][(HALMA_SQUARE_ROOT / 3)] = EMPTY;

  // intialize YELLOW pieces
  for (dimension_T i = HALMA_SQUARE_ROOT - 1;
       i >= HALMA_SQUARE_ROOT - (HALMA_SQUARE_ROOT / 3); i--)
    for (dimension_T o = HALMA_SQUARE_ROOT - 1;
         o >= HALMA_SQUARE_ROOT - (HALMA_SQUARE_ROOT / 3) - 1 +
                  (HALMA_SQUARE_ROOT - i - 1);
         o--)
      board->grid[i][o] = YELLOW;
  // remove extra pieces at end
  board->grid[HALMA_SQUARE_ROOT - 1]
             [HALMA_SQUARE_ROOT - 1 - (HALMA_SQUARE_ROOT / 3)] = EMPTY;

  // initialize the victory mask
  halma_gen_victory_mask(board);

  return board;
}

void halma_clear_moves(struct halma_board* board,
                       struct halma_moves* moves_table) {
  for (int i = 0; i < board->player_pieces; i++)
    destroy_bitmask(moves_table[i].targets);
  free(moves_table);
}
void halma_end_game(struct halma_board* board) { free(board); }