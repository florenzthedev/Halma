#ifndef HALMA_TERM_H_INCLUDED
#define HALMA_TERM_H_INCLUDED

#include "halma.h"

void halma_print_board(struct halma_board* board);
void halma_print_movable_pieces(struct halma_board* board,
                                struct halma_moves* moves);
void halma_print_targets(struct halma_board* board, struct halma_moves* move);
char halma_term_greeting();
char halma_term_game_menu(enum halma_piece turn);
char* halma_get_filename();
void halma_term_filename_perror();
void halma_no_moves_error(enum halma_piece turn);
void halma_term_victory(enum halma_piece victor);
dimension_T halma_get_game_type();
dimension_T halma_select_piece(struct halma_board* board,
                               struct halma_moves* moves,
                               enum halma_piece turn);
int halma_select_target(struct halma_board* board, struct halma_moves* move);

#endif  // HALMA_TERM_H_INCLUDED