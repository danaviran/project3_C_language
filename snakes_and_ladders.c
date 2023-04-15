#include <string.h>
#include "markov_chain.h"

#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))

#define EMPTY -1
#define BOARD_SIZE 100
#define MAX_GENERATION_LENGTH 60
#define DICE_MAX 6
#define NUM_OF_TRANSITIONS 20
#define ARG_NUM 3
#define SEED_ARG 1
#define NUM_OF_PATHS 2
#define DECIMAL 10
#define COUNTER_OF_PATHS_INIT 1
#define USAGE_ERROR "USAGE: Enter Seed & Num of wanted paths.\n"
#define ALLOC_ERROR_CELL "Allocation failure: Alloc of new cell failed."
#define ALLOCATION_ERROR_MASSAGE \
"Allocation failure: Failed to allocate new memory\n"

/**
 * represents the transitions by ladders and snakes in the game
 * each tuple (x,y) represents a ladder from x to if x<y or a snake otherwise
 */
const int transitions[][2] = {{13, 4},
                              {85, 17},
                              {95, 67},
                              {97, 58},
                              {66, 89},
                              {87, 31},
                              {57, 83},
                              {91, 25},
                              {28, 50},
                              {35, 11},
                              {8,  30},
                              {41, 62},
                              {81, 43},
                              {69, 32},
                              {20, 39},
                              {33, 70},
                              {79, 99},
                              {23, 76},
                              {15, 47},
                              {61, 14}};

/**
 * struct represents a Cell in the game board
 */
typedef struct Cell {
    int number; // Cell number 1-100
    int ladder_to;
    int snake_to;
    //both ladder_to and snake_to should be -1 if the Cell doesn't have them
} Cell;


/** Error handler **/
static int handle_error(char *error_msg, MarkovChain **database)
{
    printf("%s", error_msg);
    if (database != NULL)
    {
        free_markov_chain(database);
    }
    return EXIT_FAILURE;
}

/**
 * Creates an Array of cells that represents the board game
 * @param cells
 * @return EXIT_SUCCESS or EXIT_FAILURE
 * @ownership Strong & Weak ownership, frees data in case of failure.
 * In case of success the array is freed in fill_database function.
 */
static int create_board(Cell *cells[BOARD_SIZE])
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        cells[i] = malloc(sizeof(Cell));
        if (cells[i] == NULL)
        {
            for (int j = 0; j < i; j++) {
                free(cells[j]);
            }
            handle_error(ALLOCATION_ERROR_MASSAGE,NULL);
            return EXIT_FAILURE;
        }
        *(cells[i]) = (Cell) {i + 1, EMPTY, EMPTY};
    }
    for (int i = 0; i < NUM_OF_TRANSITIONS; i++)
    {
        int from = transitions[i][0]; // Cell of Transition
        int to = transitions[i][1];
        if (from < to)
        {
            cells[from - 1]->ladder_to = to; // Ladder
        }
        else
        {
            cells[from - 1]->snake_to = to; // Snake
        }
    }
    return EXIT_SUCCESS;
}

/**
 * fills database
 * @param markov_chain
 * @return EXIT_SUCCESS or EXIT_FAILURE
 * @ownership - Frees the Cell array allocated in create_board function
 */
static int fill_database(MarkovChain *markov_chain)
{
    Cell* cells[BOARD_SIZE];
    if(create_board(cells) == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }
    MarkovNode *from_node = NULL, *to_node = NULL;
    size_t index_to;
    for (size_t i = 0; i < BOARD_SIZE; i++)
    {
        add_to_database(markov_chain, cells[i]);
    }
    for (size_t i = 0; i < BOARD_SIZE; i++)
    {
        from_node = get_node_from_database(markov_chain,
                                           cells[i])->data;
        // If it's a cell of transition
        if (cells[i]->snake_to != EMPTY || cells[i]->ladder_to != EMPTY)
        {
            index_to = MAX(cells[i]->snake_to,cells[i]->ladder_to) - 1;
            to_node = get_node_from_database(markov_chain,
                                             cells[index_to])->data;
            add_node_to_counter_list(from_node, to_node,
                                     markov_chain);
        }
        else // For a regular cell we define 6 possibilities of moving ahead
        {
            for (int j = 1; j <= DICE_MAX; j++)
            {
                index_to = ((Cell*) (from_node->data))->number + j - 1;
                if (index_to >= BOARD_SIZE)
                {
                    break;
                }
                to_node = get_node_from_database(markov_chain,
                                                 cells[index_to])
                        ->data;
                add_node_to_counter_list(from_node,
                                         to_node, markov_chain);
            }
        }
    }
    for (size_t i = 0; i < BOARD_SIZE; i++) // free temp arr
    {
        free(cells[i]);
    }
    return EXIT_SUCCESS;
}

/**
 * Generates wanted number of tweets from the MarkovChain
 * @param markov_chain_p
 * @param num_of_tweets
 */
static void generate_tweets(MarkovChain *markov_chain_p, int max_move_num)
{
  int path_counter = COUNTER_OF_PATHS_INIT;
  while (path_counter <= max_move_num)
    {
      Node *first_node = markov_chain_p->database->first;
      printf ("\nRandom Walk %d: ", path_counter);
      generate_random_sequence (markov_chain_p,first_node
      ->data,MAX_GENERATION_LENGTH);
      path_counter++;
    }
  printf ("\n");
}

/**
 * Cell format print function to use in generic database
 * @param cell
 */
static void print_cell(Cell *cell)
{
  if (cell->ladder_to != EMPTY)
    {
      printf ("[%d]-ladder to %d -> ",cell->number, cell->ladder_to);
    }
  else if (cell->snake_to != EMPTY)
    {
      printf ("[%d]-snake to %d -> ",cell->number, cell->snake_to);
    }
  else if (cell->number != BOARD_SIZE)
    {
      printf ("[%d] -> ",cell->number);
    }
  else if (cell->number == BOARD_SIZE)
    {
      printf ("[%d]",cell->number);
    }
}

/**
 * Cell deep copy function to use in generic database
 * @param cell
 * @return copied cell
 * @ownership Weak Ownership - separate function for Free (free_markov_chain)
 */
static Cell* cell_copy(Cell *cell)
{
  if (!cell)
    {
      return NULL;
    }
  Cell *new_cell = calloc(1,sizeof (Cell));
  if (!new_cell)
    {
      printf ("%s", ALLOC_ERROR_CELL);
      return NULL;
    }
  *new_cell = (Cell) {cell->number, cell->ladder_to,
                      cell->snake_to};
  return new_cell;
}

/**
 * Check if cell is last (if cell number is MAX)
 * @param cell
 * @return true if last, false if isn't
 */
static bool is_cell_last(Cell *cell)
{
  if (cell == NULL)
    {
      return false;
    }
  if (cell->number == BOARD_SIZE)
    {
      return true;
    }
  return false;
}

/**
 * Free Cell allocation to use in generic database
 * @param cell
 */
static void free_cell(Cell *cell)
{
  if (cell)
    {
      free(cell);
      cell = NULL;
    }
}

/**
 * Compare cell function - checks if cells have the same number
 * @param cell_1
 * @param cell_2
 * @return 0 if they have same number, otherwise, the difference between them
 */
static int compare_cells(Cell *cell_1, Cell *cell_2)
{
  int result = (cell_1->number)-(cell_2->number);
  return result;
}

/**
 * MarkovChain and LinkedList initialization
 * @param database_pp
 * @param markov_chain_pp
 * @return EXIT_SUCCESS or EXIT_FAILURE
 * @ownership Strong & Weak - frees LinkedList in case of failure.
 * In case of success, separate function for Free (free_markov_chain)
 */
static int initialize_structs(LinkedList **database_pp, MarkovChain
**markov_chain_pp)
{
  *database_pp = new_linked_list();
  if (*database_pp==NULL)
    {
      return EXIT_FAILURE;
    }
  *markov_chain_pp = new_markov_chain();
  if (*markov_chain_pp == NULL)
    {
      free(*database_pp);
      return EXIT_FAILURE;
    }
  (*markov_chain_pp)->print_func = (void*) print_cell;
  (*markov_chain_pp)->copy_func = (void*) cell_copy;
  (*markov_chain_pp)->comp_func = (void*) compare_cells;
  (*markov_chain_pp)->is_last = (void*) is_cell_last;
  (*markov_chain_pp)->free_data = (void*) free_cell;
  return EXIT_SUCCESS;
}

/**
 * @param argc num of arguments
 * @param argv 1) Seed 2) Number of sentences to generate
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char *argv[])
{
  if (argc != ARG_NUM)
    {
      printf ("%s", USAGE_ERROR);
      return EXIT_FAILURE;
    }
  // initialize all arguments
  unsigned int seed = strtol (argv[SEED_ARG], NULL, DECIMAL);
  srand (seed); // use seed argument
  int num_of_paths = (int) strtol (argv[NUM_OF_PATHS], NULL,
                                   DECIMAL);
  // initialization and allocation of all structs
  LinkedList *database_p = NULL;
  MarkovChain *markov_chain_p = NULL;
  int check_success = initialize_structs (&database_p,
                                          &markov_chain_p);
  if (check_success == EXIT_FAILURE)
    {
      return EXIT_FAILURE;
    }
  markov_chain_p->database = database_p;
  check_success = fill_database (markov_chain_p);
  if (check_success == EXIT_FAILURE)
    {
      free_markov_chain(&markov_chain_p);
      return EXIT_FAILURE;
    }
  // tweet generation
  generate_tweets (markov_chain_p, num_of_paths);
  free_markov_chain(&markov_chain_p);
  return EXIT_SUCCESS;
}
