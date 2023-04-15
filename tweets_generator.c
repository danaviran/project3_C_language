#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "markov_chain.h"

#define ARG_MIN_NUM 4
#define ARG_MAX_NUM 5
#define SEED_ARG 1
#define TWEETS_NUM 2
#define TWEET_FILE 3
#define NUM_OF_WORDS 4
#define MAX_ROW 1000
#define MAX_WORD 100
#define DOT_ASCII 46
#define MAX_WORDS_IN_TWEETS 20
#define DECIMAL 10
#define NUM_OF_TWEET 1
#define MAX_INT 2147483647
#define FILE_ERROR "ERROR: problem with opening file.\n"
#define ALLOC_CHAR_ERROR \
"Allocation failure: Alloc of new char array failed.\n"
#define USAGE_ERROR \
"USAGE: Enter Seed, Tweet Num, File url & Num of words to read (optional).\n"

/**
 * Insert MarkovNode of word2 to NextNodeCounter array of word_1
 * @param list MarkovChain
 * @param word_1
 * @param word_2
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
static int insert_to_counter_list(MarkovChain list, char *word_1, char *word_2)
{
  Node *node_word1 = get_node_from_database (&list,word_1);
  Node *node_word2 = get_node_from_database (&list,word_2);
  if ((node_word1 != NULL) && (node_word2 != NULL))
    {
      bool success = add_node_to_counter_list(node_word1->data,
                           node_word2->data, &list);
      if (success == false)
        {
          return EXIT_FAILURE;
        }
      return EXIT_SUCCESS;
    }
  return EXIT_FAILURE;
}

/**
 * Insert word to database
 * @param list MarkovChain
 * @param word
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
static int insert_single_word_to_db(MarkovChain *list, char *word)
{
  Node *node_p = add_to_database (list, word);
  if (node_p == NULL)
    {
      return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

/**
 * Checks if string is marked by \n, \r or \0
 * @param word char*
 * @return true if word is marked, false if not
 */
static bool is_str_marked(char* word)
{
  int last_index = (int) strlen (word) - 1;
  if ((word[last_index] == '\n') || (word[last_index] == '\0') ||
   (word[last_index] == '\r'))
    {
      return true;
    }
  return false;
}

/**
 * Reads from the file the wanted number of words and inserts them in the
 * MarkovChain database.
 * @param fp
 * @param words_to_read
 * @param markov_chain
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
static int fill_database(FILE *fp, int words_to_read, MarkovChain *markov_chain
)
{
  int num_of_read_words = 0;
  char single_line[MAX_ROW];
  while (fgets (single_line,MAX_ROW,fp) != NULL
  && (num_of_read_words < words_to_read))
    {
      char *line = " ";
      char *word_1 = strtok(single_line,line);
      char new_word[MAX_WORD] = {0};
      int success = insert_single_word_to_db (markov_chain, word_1);
      if (success==EXIT_FAILURE)
        {
          return EXIT_FAILURE;
        }
      num_of_read_words++; // Loop all the other words in current line
      while (word_1 != NULL && (num_of_read_words < words_to_read))
        {
          char *word_2 = strtok (NULL, line);
          if (word_2 == NULL)
            {
              break;
            }
          while (is_str_marked(word_2)) // clean word from \* marks
            {
              for (int i=0; i < (int) strlen (word_2)-1;i++)
                {
                  new_word[i] = word_2[i];
                }
              word_2 = new_word;
            }
          success = insert_single_word_to_db(markov_chain, word_2);
          if (success == EXIT_FAILURE)
            {
              return EXIT_FAILURE;
            }
          num_of_read_words++;
          if (word_1[strlen (word_1)-1] != DOT_ASCII)
            {
              bool b_success = insert_to_counter_list
                  (*markov_chain, word_1, word_2);
              if (b_success == EXIT_FAILURE)
                {
                  return EXIT_FAILURE;
                }
            }
          word_1 = word_2;
        }
    }
  return EXIT_SUCCESS;
}

/**
 * Generates wanted number of tweets from the MarkovChain
 * @param markov_chain_p
 * @param num_of_tweets
 */
static void generate_tweets(MarkovChain *markov_chain_p, int num_of_tweets)
{
  int count_tweets = NUM_OF_TWEET;
  while (count_tweets <= num_of_tweets)
    {
      MarkovNode *first_node = get_first_random_node
          (markov_chain_p);
      if (first_node != NULL)
        {
          printf ("Tweet %d: ", count_tweets);
          generate_random_sequence (markov_chain_p,first_node,
                                    MAX_WORDS_IN_TWEETS);
          printf ("\n");
          count_tweets++;
        }
    }
}

/**
 * Check if the word is considered last to use in generic database
 * @param str
 * @return True if last, False if is not
 */
static bool str_is_last(char* str)
{
  if (str == NULL)
    {
      return false;
    }
  if (str[strlen (str)-1] == DOT_ASCII)
    {
      return true;
    }
  return false;
}

/**
 * Regular string print function to use in generic database
 * @param data
 */
static void print_char_func(char *data)
{
  if (data[strlen (data)-1]==DOT_ASCII)
    {
      printf ("%s",data);
    }
  else
    {
      printf("%s ",data);
    }
}

/**
 * String copy function to use in generic database
 * @param str
 * @return char* - copied string
 * @ownership Weak Ownership - separate function for Free (free_markov_chain)
 */
static char* str_copy(char* str)
{
  if (!str)
    {
      return NULL;
    }
  char *new_str = malloc (sizeof (char) * strlen (str)+1);
  if (!new_str)
    {
      printf ("%s", ALLOC_CHAR_ERROR);
      return NULL;
    }
  strcpy (new_str, str);
  return new_str;
}

/**
 * Free string allocation to use in generic database
 * @param str
 */
static void str_free(char* str)
{
  if (str)
    {
      free(str);
      str = NULL;
    }
}

/**
 * MarkovChain and LinkedList initialization
 * @param database_pp
 * @param markov_chain_pp
 * @ownership Strong & Weak - frees LinkedList in case of failure.
 * In case of success, separate function for Free (free_markov_chain)
 * @return EXIT_SUCCESS or EXIT_FAILURE
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
      free (*database_pp);
      return EXIT_FAILURE;
    }
  (*markov_chain_pp)->database = *database_pp;
  (*markov_chain_pp)->print_func = (void*) print_char_func;
  (*markov_chain_pp)->copy_func = (void*) str_copy;
  (*markov_chain_pp)->comp_func = (void*) strcmp;
  (*markov_chain_pp)->is_last = (void*) str_is_last;
  (*markov_chain_pp)->free_data = (void*) str_free;
  return EXIT_SUCCESS;
}

/**
 * @param argc num of arguments
 * @param argv 1) Seed 2) Number of sentences to generate 3) File name
 * 4) Number of words to read from file (optional)
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char **argv)
{
  if (argc != ARG_MIN_NUM && argc != ARG_MAX_NUM)
    {
      printf ("%s", USAGE_ERROR);
      return EXIT_FAILURE;
    }
  // Initialize all arguments
  unsigned int seed = strtol (argv[SEED_ARG], NULL, DECIMAL);
  srand (seed);
  int num_of_tweets = (int) strtol
      (argv[TWEETS_NUM], NULL, DECIMAL);
  char *path = argv[TWEET_FILE];
  int words_to_read = MAX_INT;
  if (argc != ARG_MIN_NUM)
    {
      words_to_read=(int)strtol(argv[NUM_OF_WORDS], NULL,
                                DECIMAL);
    }
  FILE *tweets_file = fopen ( path, "r"); // File handling
  if (tweets_file == NULL)
    {
      printf ("%s",FILE_ERROR);
      return EXIT_FAILURE;
    }
  // Initialization and allocation of all structs
  LinkedList *database_p = NULL;
  MarkovChain *markov_chain_p = NULL;
  int success = initialize_structs (&database_p,
                                          &markov_chain_p);
  if (success == EXIT_FAILURE)
    {
      return EXIT_FAILURE;
    }
  success = fill_database(tweets_file, words_to_read, // Learning process
                          markov_chain_p);
  fclose (tweets_file); // Strong Ownership
  if (success == EXIT_FAILURE)
    {
      free_markov_chain (&markov_chain_p);
      return EXIT_FAILURE;
    }
  generate_tweets (markov_chain_p, num_of_tweets); // Tweet generation
  free_markov_chain(&markov_chain_p);
  return EXIT_SUCCESS;
}