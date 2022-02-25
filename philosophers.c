#include <pthread.h>
#include <stdio.h>
#include <assert.h>

#define HUNGER 0x10
#define PHILOSOPHERS 0x10
#define DELAY 0x1000000

typedef struct {
  pthread_mutex_t mutex;
  unsigned int held;
  unsigned int uses;
  unsigned int times_held;
} Fork;

typedef struct {
  Fork *left_fork;
  Fork *right_fork;
  unsigned int food_consumed;
  unsigned int id;
} Philosopher;

void eat(Philosopher * philosopher) {
  /*
   * Simulates a philosopher eating. Requires both forks to be held. 
   */

  philosopher->food_consumed++;
  printf("Philosopher %d has eaten %d times.\n", philosopher->id, philosopher->food_consumed);

  assert(philosopher->left_fork->uses < philosopher->left_fork->times_held);
  assert(philosopher->left_fork->held == 1);
  assert(philosopher->right_fork->held == 1);
  philosopher->left_fork->uses++;
  philosopher->right_fork->uses++;
}

void get_fork(Fork * fork) {
  /*
   * Blocks until `fork` is available then picks it up 
   */
  for (unsigned i = 0; i < DELAY; i++);
  pthread_mutex_lock(&fork->mutex);
  assert(fork->held == 0);
  fork->held = 1;
  fork->times_held++;
}

void return_fork(Fork * fork) {
  /*
   * Puts `fork` back and never blocks 
   */
  assert(fork->held == 1);
  fork->held = 0;
  pthread_mutex_unlock(&fork->mutex);
}

long int run_philosopher(Philosopher * philosopher) {
  /*
   * Implements one dining philosopher
   *
   * The purpose of this thread is to `eat` until no longer hungry. In order 
   * to eat, a philosopher must be holding their right and left fork. The 
   * naive solution to this problem creates a deadlock. You must find a 
   * better algorithm. The ideal algorithm will still allow concurrent 
   * eating without creating deadlocks.
   *
   * Note that philosophers must return forks bewteen calls to `eat`.
   */
  while (philosopher->food_consumed < HUNGER) {
    get_fork(philosopher->left_fork);
    get_fork(philosopher->right_fork);

    eat(philosopher);

    return_fork(philosopher->right_fork);
    return_fork(philosopher->left_fork);
  }

  return philosopher->food_consumed;
}

/*
 * Setup and test code
 */

int main() {
  pthread_t child_thread[PHILOSOPHERS];

  Philosopher philosophers[PHILOSOPHERS];
  Fork forks[PHILOSOPHERS];

  for (unsigned long i = 0; i < PHILOSOPHERS; i++) {
    pthread_mutex_init(&forks[i].mutex, 0);

    forks[i].held = 0;
    forks[i].uses = 0;
    forks[i].times_held = 0;

    philosophers[i].id = i;
    philosophers[i].food_consumed = 0;
    philosophers[i].left_fork = &forks[i];
    philosophers[i].right_fork = &forks[(i + 1) % PHILOSOPHERS];

    int code;
    code = pthread_create(&child_thread[i], NULL, (void *) run_philosopher, (void *) &philosophers[i]);
    if (code) {
      fprintf(stderr, "pthread_create failed with code %d\n", code);
    }
  }

  unsigned long thread_consumed;
  unsigned long reported_total_consumed = 0;

  for (unsigned int i = 0; i < PHILOSOPHERS; i++) {
    pthread_join(child_thread[i], (void *) &thread_consumed);
    assert(thread_consumed == HUNGER);
    assert(philosophers[i].food_consumed == HUNGER);
    reported_total_consumed += thread_consumed;
  }

  printf("%ld total food consumed.\n", reported_total_consumed);
  assert(reported_total_consumed == HUNGER * PHILOSOPHERS);

  unsigned long total_fork_uses = 0;
  unsigned long total_fork_holds = 0;

  for (unsigned int i = 0; i < PHILOSOPHERS; i++) {
    total_fork_uses += forks[i].uses;
    total_fork_holds += forks[i].times_held;
  }

  printf("%ld total fork uses.\n", total_fork_uses);
  assert(total_fork_uses == HUNGER * PHILOSOPHERS * 2);
  assert(total_fork_holds == HUNGER * PHILOSOPHERS * 2);

  printf("All tests passed.\n");

  return 0;
}
