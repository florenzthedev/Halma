#include "halma_term.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitmask.h"

#define C_BLANK " "
#define F_RED "\033[31m*"
#define F_GREEN "\033[32m*"
#define F_BLUE "\033[34m*"
#define F_YELLOW "\033[33m*"
#define F_PURPLE "\033[35m*"
#define F_MOVE "\033[35m^"
#define B_WHITE "\033[47m"
#define B_BLACK "\033[40m"

// the longest input is likely going to be a filename for a save file.
// if your filename is longer than 4k you have some problems
#define INPUT_BUFFER_SIZE 4096
char input_buffer[INPUT_BUFFER_SIZE];

const char* piece_name[] = {"EMPTY", "RED", "YELLOW", "BLUE", "GREEN"};

#define EXIT_STDIN_ERROR 30

char halma_term_greeting() {
  char selection = '\0';
  do {
    printf("Main Menu\n[N]ew Game\n[L]oad Game\n[Q]uit\nSelection: ");
    if (!fgets(input_buffer, INPUT_BUFFER_SIZE, stdin)) {
      // something has gone very wrong, likely an issue with stdin
      perror(NULL);
      exit(EXIT_STDIN_ERROR);
    }

    if (1 != sscanf(input_buffer, " %c", &selection)) {
      // we already checked the input stdin, no need to do so again here
      printf("Invalid input format.\n");
      continue;
    }

    selection = toupper(selection);
    if (selection == 'N' || selection == 'L' || selection == 'Q')
      return selection;

    printf("Invalid selection.\n");
  } while (true);
}

char halma_term_game_menu(enum halma_piece turn) {
  char selection = '\0';
  do {
    printf("\nGame Menu\nCurrent Turn: %s\n", piece_name[turn]);
    printf(
        "Print [B]oard\nPrint Movable [P]ieces\nPrint Piece [T]argets\n[M]ake "
        "Move\n");
    printf("[S]ave Game\n[Q]uit to Main Menu\nSelection: ");
    if (!fgets(input_buffer, INPUT_BUFFER_SIZE, stdin)) {
      // something has gone very wrong, likely an issue with stdin
      perror(NULL);
      exit(EXIT_STDIN_ERROR);
    }

    if (1 != sscanf(input_buffer, " %c", &selection)) {
      // we already checked the input stdin, no need to do so again here
      printf("Invalid input format.\n");
      continue;
    }

    selection = toupper(selection);
    if (selection == 'B' || selection == 'P' || selection == 'T' ||
        selection == 'M' || selection == 'S' || selection == 'Q')
      return selection;

    printf("Invalid selection.\n");
  } while (true);
}

char* halma_get_filename() {
  printf("Enter filename: ");
  if (!fgets(input_buffer, INPUT_BUFFER_SIZE, stdin)) {
    // something has gone very wrong, likely an issue with stdin
    perror(NULL);
    exit(EXIT_STDIN_ERROR);
  }
  input_buffer[strcspn(input_buffer, "\n")] = '\0';
  return input_buffer;
}

void halma_term_filename_perror() {
  perror("There was an error loading the file");
}

dimension_T halma_get_game_type() {
  int players;
  do {
    printf("Enter game type, [2] player, [4] player: ");
    if (!fgets(input_buffer, INPUT_BUFFER_SIZE, stdin)) {
      // something has gone very wrong, likely an issue with stdin
      perror(NULL);
      exit(EXIT_STDIN_ERROR);
    }

    if (1 != sscanf(input_buffer, " %i", &players)) {
      // we already checked the input stdin, no need to do so again here
      printf("Invalid input format.\n");
      continue;
    }

    if (players != 2 && players != 4) {
      printf("Invalid game type.\n");
      continue;
    }

    return players;
  } while (true);
}

void halma_no_moves_error(enum halma_piece turn){
  printf("%s has no possible moves, turn forfeit.\n", piece_name[turn]);
}

/**
 * @brief Converts a ASCII character hex input to its value.
 * I tried to be clever with strtol but it didn't work.
 * @param hex the single character to be converted.
 * @return the value of hex, -1 if not valid.
 */
dimension_T hex_to_value(char hex) {
  hex = toupper(hex);
  switch (hex) {
    case '0':
      return 0;
    case '1':
      return 1;
    case '2':
      return 2;
    case '3':
      return 3;
    case '4':
      return 4;
    case '5':
      return 5;
    case '6':
      return 6;
    case '7':
      return 7;
    case '8':
      return 8;
    case '9':
      return 9;
    case 'A':
      return 10;
    case 'B':
      return 11;
    case 'C':
      return 12;
    case 'D':
      return 13;
    case 'E':
      return 14;
    case 'F':
      return 15;
    default:
      return -1;
  }
}

dimension_T halma_select_piece(struct halma_board* board,
                               struct halma_moves* moves,
                               enum halma_piece turn) {
  char y_coord, x_coord;
  dimension_T y_index, x_index;

  do {
    printf("Enter coordinates of %s piece to move [XY]: ", piece_name[turn]);
    if (!fgets(input_buffer, INPUT_BUFFER_SIZE, stdin)) {
      // something has gone very wrong, likely an issue with stdin
      perror(NULL);
      exit(EXIT_STDIN_ERROR);
    }

    if (2 != sscanf(input_buffer, " %c %c", &x_coord, &y_coord)) {
      // we already checked the input stdin, no need to do so again here
      printf("Invalid input format.\n");
      continue;
    }

    y_index = hex_to_value(y_coord);
    x_index = hex_to_value(x_coord);

    if (y_index == -1 || x_index == -1) {
      printf("Invalid input characters.\n");
      continue;
    }

    // check to see if given coords are usable
    dimension_T entry_validation =
        halma_validate_piece_selection(board, y_index, x_index, moves, turn);
    if (entry_validation >= 0) return entry_validation;
    switch (entry_validation) {
      case -1:
        printf("A %s piece is not present on that tile.\n", piece_name[turn]);
        continue;
      case -2:
        return -2;  // somethings gone very wrong
      case -3:
        printf("This piece does not have any possible valid moves.\n");
        return -1;
    }

    return -1;
  } while (true);
}

void halma_term_victory(enum halma_piece victor){
  printf("%s wins! Congratulations!\n", piece_name[victor]);
}

int halma_select_target(struct halma_board* board, struct halma_moves* move) {
  char y_coord, x_coord;
  dimension_T y_index, x_index;

  halma_print_targets(board, move);

  do {
    printf("Enter coordinates to move to [XY]: ");
    if (!fgets(input_buffer, INPUT_BUFFER_SIZE, stdin)) {
      // something has gone very wrong, likely an issue with stdin
      perror(NULL);
      exit(EXIT_STDIN_ERROR);
    }

    if (2 != sscanf(input_buffer, " %c%c", &x_coord, &y_coord)) {
      // we already checked the input stdin, no need to do so again here
      printf("Invalid input format.\n");
      continue;
    }

    y_index = hex_to_value(y_coord);
    x_index = hex_to_value(x_coord);

    if (y_index == -1 || x_index == -1) {
      printf("Invalid input characters.\n");
      continue;
    }

    if (!halma_validate_target_selection(move, y_index, x_index)) {
      printf("That is not a valid tile to move to.\n");
      continue;
    }

    // Passing a 2D coordinate pair as a single integer
    return (y_index * HALMA_SQUARE_ROOT) + x_index;
  } while (true);
}

// This resets it to default for the newline otherwise
// the last color would continue to the end of the line
#define H_NEWLINE "\033[0m\n"

void halma_print_board(struct halma_board* board) {
  printf(" ");  // Space for left column of numbers
  for (dimension_T i = 0; i < HALMA_SQUARE_ROOT; i++) printf("%1X", i);
  printf("\n");
  for (dimension_T i = 0; i < HALMA_SQUARE_ROOT; i++) {
    printf("%1X", i);
    for (dimension_T o = 0; o < HALMA_SQUARE_ROOT; o++) {
      if ((i + o) % 2)
        printf(B_WHITE);
      else
        printf(B_BLACK);
      switch (board->grid[i][o]) {
        case RED:
          printf(F_RED);
          break;
        case GREEN:
          printf(F_GREEN);
          break;
        case BLUE:
          printf(F_BLUE);
          break;
        case YELLOW:
          printf(F_YELLOW);
          break;
        default:
          printf(C_BLANK);
          break;
      }
    }
    printf(H_NEWLINE);
  }
}

bool halma_is_coord_movable(struct halma_board* board,
                            struct halma_moves* moves, dimension_T y_index,
                            dimension_T x_index) {
  for (dimension_T i = 0; i < board->player_pieces; i++)
    if (moves[i].origin_y == y_index && moves[i].origin_x == x_index) 
      if(anybit(moves[i].targets)) return true;
  return false;
}

void halma_print_movable_pieces(struct halma_board* board,
                                struct halma_moves* moves) {
  printf(" ");  // Space for left column of numbers
  for (dimension_T i = 0; i < HALMA_SQUARE_ROOT; i++) printf("%1X", i);
  printf("\n");
  for (dimension_T i = 0; i < HALMA_SQUARE_ROOT; i++) {
    printf("%1X", i);
    for (dimension_T o = 0; o < HALMA_SQUARE_ROOT; o++) {
      if ((i + o) % 2)
        printf(B_WHITE);
      else
        printf(B_BLACK);
      if (halma_is_coord_movable(board, moves, i, o)) {
        printf(F_PURPLE);
      } else {
        switch (board->grid[i][o]) {
          case RED:
            printf(F_RED);
            break;
          case GREEN:
            printf(F_GREEN);
            break;
          case BLUE:
            printf(F_BLUE);
            break;
          case YELLOW:
            printf(F_YELLOW);
            break;
          default:
            printf(C_BLANK);
            break;
        }
      }
    }
    printf(H_NEWLINE);
  }
}

void halma_print_targets(struct halma_board* board, struct halma_moves* move) {
  printf(" ");  // Space for left column of numbers
  for (dimension_T i = 0; i < HALMA_SQUARE_ROOT; i++) printf("%1X", i);
  printf("\n");
  for (dimension_T i = 0; i < HALMA_SQUARE_ROOT; i++) {
    printf("%1X", i);
    for (dimension_T o = 0; o < HALMA_SQUARE_ROOT; o++) {
      if ((i + o) % 2)
        printf(B_WHITE);
      else
        printf(B_BLACK);
      if (getbit(move->targets, i, o)) {
        printf(F_MOVE);
      } else if (i == move->origin_y && o == move->origin_x) {
        printf(F_PURPLE);
      } else {
        switch (board->grid[i][o]) {
          case RED:
            printf(F_RED);
            break;
          case GREEN:
            printf(F_GREEN);
            break;
          case BLUE:
            printf(F_BLUE);
            break;
          case YELLOW:
            printf(F_YELLOW);
            break;
          default:
            printf(C_BLANK);
            break;
        }
      }
    }
    printf(H_NEWLINE);
  }
}
