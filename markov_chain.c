#ifndef _MARKOV_CHAIN_C
#define _MARKOV_CHAIN_C

#include "markov_chain.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define ALLOC_ERROR_MARKOV_CHAIN \
"Allocation failure: Alloc of new MarkovChain failed.\n"
#define ALLOC_ERROR_LINKEDLIST \
"Allocation failure: Alloc of new LinkedList failed.\n"
#define ALLOC_ERROR_NODE "Allocation failure: Alloc of new Node failed.\n"
#define ALLOC_ERROR_MARKOV_NODE \
"Allocation failure: Alloc of new MarkovNode failed.\n"
#define ALLOC_ERROR_GENERIC_DATA \
"Allocation failure: Alloc of generic data failed.\n"
#define ALLOC_ERROR_COUNTER_LIST \
"Allocation failure: Alloc of counter_list failed.\n"
#define ALLOC_ERROR_REALLOC_COUNTER_LIST \
"Allocation failure: reallocation of NextNodeCounter array failed.\n"


/**
* Get random number between 0 and max_number [0, max_number).
* @param max_number maximal number to return (not including)
* @return Random number
*/
int get_random_number(int max_number)
{
  return rand() % max_number;
}

/**
 * Get one random state from the given markov_chain's database.
 * @param markov_chain
 * @return MarkovNode pointer, first random word
 */
MarkovNode* get_first_random_node(MarkovChain *markov_chain)
{
  while (true)
    {
      int index = get_random_number (markov_chain->database->size);
      Node *ptr = markov_chain->database->first;
      for (int i =0;i<index;i++)
        {
          ptr = ptr->next;
        }
      if (ptr->data->counter_list_length != 0)
        {
          return ptr->data;
        }
    }
}

/**
 * Choose randomly the next state, depend on it's occurrence frequency.
 * @param state_struct_ptr MarkovNode to choose from
 * @return MarkovNode of the chosen state
 */
MarkovNode* get_next_random_node(MarkovNode *state_struct_ptr)
{
  int index = get_random_number(state_struct_ptr->
      counter_list_total);
  int i = 0;
  while (i < (state_struct_ptr->counter_list_length))
    {
      index -= state_struct_ptr->counter_list[i].frequency;
      if (index < 0)
       {
          return (state_struct_ptr->counter_list[i]).markov_node;
       }
      i++;
    }
  return NULL;
}

/**
 * Receive markov_chain, generate and print random sequence out of it. The
 * sequence most have at least 2 data elements in it.
 * @param markov_chain
 * @param first_node markov_node to start with,
 *                   if NULL- choose a random markov_node
 * @param  max_length maximum length of chain to generate
 */
void generate_random_sequence(MarkovChain *markov_chain,
                              MarkovNode *first_node, int max_length)
{
  if (markov_chain != NULL)
    {
      MarkovNode *cur_node = first_node;
      markov_chain->print_func(cur_node->data);
      int i = 1;
      while (i < max_length)
        {
          cur_node = get_next_random_node(cur_node);
          if (cur_node->counter_list_total == 0)
            {
              markov_chain->print_func(cur_node->data);
              break;
            }
          else
            {
              markov_chain->print_func(cur_node->data);
              i++;
            }
        }
    }
}

/**
 * Initialize and Allocate new MarkovChain
 * @return MarkovChain pointer
 * returns NULL in case of memory allocation failure.
 * @ownership Weak Ownership. separate function for Free (free_markov_chain)
 */
MarkovChain* new_markov_chain()
 {
  MarkovChain* new_markov_chain = malloc (sizeof (MarkovChain));
  if (new_markov_chain == NULL)
    {
      printf ("%s", ALLOC_ERROR_MARKOV_CHAIN);
    }
  else
    {
      new_markov_chain->database = NULL;
    }
  return new_markov_chain;
 }

 /**
  * Initialize and Allocate new LinkedList
  * @return LinkedList pointer
  * returns NULL in case of memory allocation failure.
  * @ownership Weak Ownership. separate function for Free (free_markov_chain)
  */
LinkedList* new_linked_list()
{ // Weak Ownership - separate  function for Free (free_markov_chain)
  LinkedList* new_linked_list = malloc (sizeof (LinkedList));
  if (new_linked_list == NULL)
    {
      printf ("%s", ALLOC_ERROR_LINKEDLIST);
    }
  else
    {
      *new_linked_list = (LinkedList) {NULL, NULL, 0};
    }
  return new_linked_list;
}

/**
 * Initialize and Allocate new LinkedList
 * @return LinkedList pointer
 * returns NULL in case of memory allocation failure.
 * @ownership Weak Ownership. separate function for Free (free_markov_chain)
 */
NextNodeCounter* new_counter_list(MarkovNode *markov_node)
{
  markov_node->counter_list = malloc (sizeof (NextNodeCounter));
  if(markov_node->counter_list == NULL)
    {
      printf ("%s", ALLOC_ERROR_COUNTER_LIST);
    }
  return markov_node->counter_list;
}

/**
 * Initialize and Allocate new MarkovNode
 * @param data generic data
 * @return MarkovNode pointer
 * returns NULL in case of memory allocation failure.
 * @ownership Weak Ownership. separate function for Free (free_markov_chain)
 */
MarkovNode* new_markov_node()
{
  MarkovNode *new_markov_node;
  new_markov_node = malloc (sizeof (MarkovNode));
  if (new_markov_node == NULL)
    {
      printf ("%s", ALLOC_ERROR_MARKOV_NODE);
      return NULL;
    }
  *new_markov_node = (MarkovNode) {NULL, NULL, 0,
                                   0};
  return new_markov_node;
}

int new_generic_data(void *data, MarkovNode *markov_node, MarkovChain
*markov_chain)
{
  void *cur_data = markov_chain->copy_func(data); //data allocated in copy_func
  if (cur_data == NULL)
    {
      printf ("%s", ALLOC_ERROR_GENERIC_DATA);
      return EXIT_FAILURE;
    }
  else
    {
      markov_node->data = cur_data;
      return EXIT_SUCCESS;
    }
}

/**
 * Free MarkovNode and all it's members
 * @param markov_chain_p
 * @param markov_node_p
 */
void free_markov_node(MarkovChain *markov_chain_p, MarkovNode *markov_node_p)
{
  (markov_chain_p)->free_data(markov_node_p->data); // free generic data
  if (markov_node_p->counter_list != NULL) // if counter_list is not NULL
    {
      free( markov_node_p->counter_list);
    }
  free (markov_node_p); // free MarkovNode
}

/**
 * Free markov_chain and all of it's content from memory
 * @param markov_chain markov_chain to free
 */
void free_markov_chain(MarkovChain ** ptr_chain)
{
  Node *cur_node = (*ptr_chain)->database->first;
  while (cur_node != NULL) // go over all database
    { // free data (MarkovNode) of each Node
      free_markov_node (*ptr_chain, cur_node->data);
      cur_node = cur_node->next;
    }
  cur_node = (*ptr_chain)->database->first;
  while (cur_node != NULL) // go over all database
  { // free Nodes of LinkedList
      Node *ptr_helper = cur_node;
      cur_node = cur_node->next;
      free (ptr_helper);
      ptr_helper = NULL;
    }
  free((*ptr_chain)->database); // free database
  free(*ptr_chain); // free markov_chain
  *ptr_chain = NULL;
}

/**
 * Add the second markov_node to the counter list of the first markov_node.
 * If already in list, update it's counter value.
 * @param first_node
 * @param second_node
 * @param markov_chain
 * @return success/failure: true if the process was successful, false if in
 * case of allocation error.
 */
bool add_node_to_counter_list(MarkovNode *first_node, MarkovNode *second_node,
                              MarkovChain *markov_chain)
{
  if (first_node->counter_list == NULL)
    {
      first_node->counter_list = new_counter_list(first_node);
      if (first_node->counter_list == NULL)
        {
          return false;
        }
    }
  else // if NextNodeCounter counter_list is already initialized
    { // Check if data is already in NextNodeCounter counter_list
      for (int i = 0; i < first_node->counter_list_length; i++)
        {
          if ((markov_chain->comp_func(first_node->counter_list[i].
          markov_node->data,second_node->data))==0)
            {
              first_node->counter_list[i].frequency++;
              first_node->counter_list_total++;
              return true;
            }
        } // If it is not, reallocate counter_list and add it
      NextNodeCounter* success = realloc(first_node->counter_list
          ,(sizeof(NextNodeCounter)*(first_node->counter_list_length+1)));
      if (success == NULL)
        {
          printf ("%s", ALLOC_ERROR_REALLOC_COUNTER_LIST);
          return false;
        }
      first_node->counter_list = success;
    }
  NextNodeCounter second=(NextNodeCounter){second_node,1};
  first_node->counter_list[first_node->counter_list_length] = second;
  first_node->counter_list_length++;
  first_node->counter_list_total++;
  return true;
}

/**
* Check if data_ptr is in database. If so, return the Node wrapping it in
 * the markov_chain, otherwise return NULL.
 * @param markov_chain the chain to look in its database
 * @param data_ptr the state to look for
 * @return Pointer to the Node wrapping given state, NULL if state not in
 * database.
 */
Node* get_node_from_database(MarkovChain *markov_chain, void *data_ptr)
{
  if (markov_chain->database->first != NULL)
    {
      Node *ptr = markov_chain->database->first;
      for (;ptr!=NULL;ptr=ptr->next)
        {
        if ((markov_chain->comp_func(ptr->data->data, data_ptr))==0)
          {
            return ptr;
          }
        }
    }
  return NULL;
}

/**
* If data_ptr in markov_chain, return it's Node. Otherwise, create new
 * Node, add to end of markov_chain's database and return it.
 * @param markov_chain the chain to look in its database
 * @param data_ptr the state to look for
 * @return Node wrapping given data_ptr in given chain's database,
 * returns NULL in case of memory allocation failure.
 */
Node* add_to_database(MarkovChain *markov_chain, void *data_ptr)
{
   Node *is_found = get_node_from_database (markov_chain, data_ptr);
   if (is_found != NULL)
     {
       return is_found;
     }
  MarkovNode *new_markov = new_markov_node ();
  if (new_markov == NULL)
    {
      return NULL;
    }
  int success = new_generic_data(data_ptr, new_markov,
                                 markov_chain);
  if (success == EXIT_FAILURE)
    {
      free(new_markov);
      return NULL;
    }
  success = add (markov_chain->database, new_markov);
  if (success == EXIT_FAILURE)
    {
      printf ("%s", ALLOC_ERROR_NODE);
      free_markov_node (markov_chain, new_markov);
      return NULL;
    }
  return markov_chain->database->last;
}

#endif