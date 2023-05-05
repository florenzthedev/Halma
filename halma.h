#ifndef HALMA_H_INCLUDED
#define HALMA_H_INCLUDED
#include "bitmask.h"

#define HALMA_SQUARE_ROOT 16

// dimension_T is used for anything thats around the same magnitude as the board
// pieces, it is signed on purpose
// in hindsight this might have become a little bit overused
typedef signed char dimension_T;

enum halma_piece { EMPTY = 0, RED = 1, YELLOW = 2, BLUE = 3, GREEN = 4 };

/**
 * @brief Structure that holds a bitmask and coordiante data for valid moves a
 * piece can make. If a function wants the array it will ask for moves or table,
 * if it wants a single instance of the struct it will ask for move. This is
 * something I would like to improve in a later version.
 */
struct halma_moves {
  bitmask_T* targets;
  dimension_T origin_y;
  dimension_T origin_x;
};

struct halma_board {
  short turns;
  enum halma_piece grid[HALMA_SQUARE_ROOT][HALMA_SQUARE_ROOT];
  enum halma_piece victory_mask[HALMA_SQUARE_ROOT][HALMA_SQUARE_ROOT];
  dimension_T player_pieces;
  dimension_T players;
};

/**
 * @brief Allocates a new halma board on the heap and initializes the pieces for
 * a 4 player game.
 *
 * @return struct halma_board*, with the pieces neatly arranged.
 */
struct halma_board* halma_init_board_4p();

/**
 * @brief Allocates a new halma board on the heap and initializes the pieces for
 * a 2 player game.
 *
 * @return struct halma_board*, with the pieces neatly arranged.
 */
struct halma_board* halma_init_board_2p();

/**
 * @brief Saves the current game to a file for loading later. The save file is a
 * binary format, the contents of board are written to the file as-is.
 * Everything but the number of turns is a single byte so endianness is not
 * generally a concern. Turns has some math done to it to handle poitential
 * endian hiccups.
 *
 * @param filename the name of the file to write to.
 * @param board the board to save to file.
 * @return int 0 on success, 1 otherwise.
 */
int halma_save_game(const char* filename, struct halma_board* board);

/**
 * @brief Loads a game from file, allocating a new board on the heap to put it
 * in.
 *
 * @param filename the name of the file to load from.
 * @return struct halma_board* the loaded board, NULL if there was an issue.
 */
struct halma_board* halma_load_game(const char* filename);

/**
 * @brief Allocates an array of struct halma_moves with board->player_pieces
 * number of elements. Each instance of halma_moves contains the current
 * coordinates of the piece and a bitmask (like a 2D bitfield, see bitmask.h)
 * that contains an entry for every tile on the board. 0 if the piece cannot
 * move there, 1 if it can. Array is allocated on the heap.
 *
 * @param board current game board.
 * @param set which player/set of pieces are we gathering the moves for.
 * @return struct halma_moves* pointer to array.
 */
struct halma_moves* halma_gather_valid_moves(struct halma_board* board,
                                             enum halma_piece set);

/**
 * @brief Deallocates the array of struct halma_moves that was allocated by
 * halma_gather_valid_moves.
 *
 * @param board game board array was allocated for.
 * @param moves_table array to deallocate.
 */
void halma_clear_moves(struct halma_board* board,
                       struct halma_moves* moves_table);

/**
 * @brief Validates a piece selection and finds the index for that piece in the
 * move table if its valid.
 *
 * @param board the current game board.
 * @param y_index the y coordinate entry to be validated.
 * @param x_index the x coordinate entry to be validated.
 * @param moves the full moves table.
 * @param turn the current player/set who's turn it is.
 * @return dimension_T:
 * -1 if a piece of type turn is not found at the give coordinates.
 * -2 if a piece was found but no entry for it was found in the move table.
 * -3 if a piece has no valid moves.
 * Otherwise returns the index in the move table for the selected piece.
 */
dimension_T halma_validate_piece_selection(struct halma_board* board,
                                           dimension_T y_index,
                                           dimension_T x_index,
                                           struct halma_moves* moves,
                                           enum halma_piece turn);

/**
 * @brief Validates the target for a given piece. Much simpler than checking if
 * a piece is good.
 *
 * @param move move table entry for the piece we are moving.
 * @param y_index the y coordinate entry to be validated.
 * @param x_index the x coordinate entry to be validated.
 * @return true if entry is valid.
 * false if entry is invalid.
 */
bool halma_validate_target_selection(struct halma_moves* move,
                                     dimension_T y_index, dimension_T x_index);

/**
 * @brief Accepts the given move into the board and increments the number of
 * turns. It does validate the move.
 *
 * @param board the current game board.
 * @param move entry in the move table for the piece we are moving.
 * @param y_index target y coordinate.
 * @param x_index target x coordinate.
 * @return dimension_T 0 on success, -1 if the move was not valid, -2 if turns
 * overflow (how??).
 */
dimension_T halma_accept_move(struct halma_board* board,
                              struct halma_moves* move, dimension_T y_index,
                              dimension_T x_index);

/**
 * @brief Checks to see if there are ANY moves available for the current
 * player/set. It should be very very rare that there are no moves available but
 * its technically possible.
 *
 * @param board the current game board.
 * @param moves full move table.
 * @return true if there are possible moves, false otherwise.
 */
bool halma_any_possible_moves(struct halma_board* board,
                              struct halma_moves* moves);

/**
 * @brief Checks to see if anyone has won. It does check how many players
 * there are and doesn't waste time on pieces not on the board.
 *
 * @param board the current game board.
 * @return enum halma_piece, the player/set that won, EMPTY if nobody has
 * won yet.
 */
enum halma_piece halma_check_victory_all(struct halma_board* board);

/**
 * @brief locates who's turn it is, all of the needed information is inside of
 * halma_board.
 *
 * @param board the current game board.
 * @return enum halma_piece, the piece who's turn it is.
 */
enum halma_piece halma_whos_turn(struct halma_board* board);

/**
 * @brief Deallocates a halma_board.
 *
 * @param board Board to deallocate.
 */
void halma_end_game(struct halma_board* board);

#endif  // HALMA_H_INCLUDED