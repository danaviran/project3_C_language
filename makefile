PHONY: clean

CCFLAGS = -std=c99 -Wall -Wextra -Wvla

tweets:
	gcc $(CCFLAGS) tweets_generator.c markov_chain.h markov_chain.c linked_list.h linked_list.c -o tweets_generator

snake:
	gcc $(CCFLAGS) snakes_and_ladders.c markov_chain.h markov_chain.c linked_list.h linked_list.c -o snakes_and_ladders

tweets_test:
	gcc -g -o ./tweets_generator_test -Wl,--wrap=malloc -Wl,--wrap=rand -Wl,--wrap=srand -DTWEETS tweets_generator.c markov_chain.c linked_list.c
snake_test:
	gcc -g -o ./snakes_and_ladders_test -Wl,--wrap=malloc -Wl,--wrap=rand -Wl,--wrap=srand -DSNAKE snakes_and_ladders.c markov_chain.c linked_list.c

clean:
	rm *.o *.exe
