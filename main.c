#include <stdio.h>

#include "halma.h"
#include "halma_term.h"

int main() {
  struct halma_board* board = NULL;
  struct halma_moves* moves = NULL;
  enum halma_piece turn = EMPTY;
  dimension_T move_index = 0;
  int target_composite = 0;
  char* filename = NULL;
  bool gameloop, refreshmoves;

  // main program loop
  do {
    // main menu
    switch (halma_term_greeting()) {
      case 'N':
        if (halma_get_game_type() == 2)
          board = halma_init_board_2p();
        else
          board = halma_init_board_4p();
        break;
      case 'L':
        filename = halma_get_filename();
        board = halma_load_game(filename);
        if (board == NULL) {
          halma_term_filename_perror();
          continue;
        }
        break;
      case 'Q':
        return 0;
      default:
        return 1;
    }

    // gameplay loop
    gameloop = true;
    refreshmoves = true;
    do {
      turn = halma_whos_turn(board);
      if (refreshmoves) {
        moves = halma_gather_valid_moves(board, turn);
        if(!halma_any_possible_moves(board,moves))
        {
          halma_clear_moves(board,moves);
          moves = NULL;
          halma_no_moves_error(turn);
        }
        refreshmoves = false;
      }
      switch (halma_term_game_menu(turn)) {
        case 'B':
          halma_print_board(board);
          break;

        case 'P':
          halma_print_movable_pieces(board, moves);
          break;

        case 'T':
          move_index = halma_select_piece(board, moves, turn);
          if (move_index < 0) break;
          halma_print_targets(board, &moves[move_index]);
          break;

        case 'M':
          move_index = halma_select_piece(board, moves, turn);
          if (move_index < 0) break;
          target_composite = halma_select_target(board, &moves[move_index]);
          halma_accept_move(board, &moves[move_index],
                            target_composite / HALMA_SQUARE_ROOT,
                            target_composite % HALMA_SQUARE_ROOT);
          refreshmoves = true;
          halma_print_board(board);
          break;

        case 'S':
          filename = halma_get_filename();
          halma_save_game(filename, board);
          break;

        case 'Q':
          gameloop = false;
          refreshmoves = true;
          break;
        default:
          return 2;
      }

      if (refreshmoves) {
        halma_clear_moves(board, moves);
        moves = NULL;

        //we only need to check if someones won after a move
        turn = halma_check_victory_all(board);
        if(turn != EMPTY)
        {
          halma_term_victory(turn);
          gameloop = false;
        }
      }
      
    } while (gameloop);

    halma_end_game(board);
    board = NULL;
  } while (true);
  return 0;
}