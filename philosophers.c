#include <pthread.h>
#include <stdio.h>
#include <assert.h>

#define HUNGER 0x10
#define PHILOSOPHERS 0x10
#define PICKUP_TIME 0x1000000

typedef struct {
  pthread_mutex_t mutex;
  unsigned int held;
  unsigned int uses;
} Fork;

typedef struct {
  Fork* left_fork;
  Fork* right_fork;
  unsigned int food_consumed;
  unsigned int id;
} Philosopher;

void eat(Philosopher * philosopher) {
  philosopher->food_consumed++;

  assert(philosopher->left_fork->held == 1);
  assert(philosopher->right_fork->held == 1);

  printf("Philosopher %d has eaten %d times.\n", philosopher->id, philosopher->food_consumed);
}

void get_fork(Fork * fork) {
    for (int i = 0; i < PICKUP_TIME; i++);
    pthread_mutex_lock(&fork->mutex);
    assert(fork->held == 0);
    fork->held = 1;
}

void return_fork(Fork * fork) {
    assert(fork->held == 1);
    fork->held = 0;
    pthread_mutex_unlock(&fork->mutex);
}

long int child(Philosopher * philosopher) {
  while (philosopher->food_consumed < HUNGER) {
    get_fork(philosopher->left_fork);
    get_fork(philosopher->right_fork);

    eat(philosopher);

    return_fork(philosopher->right_fork);
    return_fork(philosopher->left_fork);
  }

  return philosopher->food_consumed;
}

int main() {
  pthread_t child_thread[PHILOSOPHERS];

  Philosopher philosophers[PHILOSOPHERS];
  Fork forks[PHILOSOPHERS];

  for (unsigned long i = 0; i < PHILOSOPHERS; i++) {
    pthread_mutex_init(&forks[i].mutex, 0);

    forks[i].held = 0;

    philosophers[i].id = i;
    philosophers[i].food_consumed = 0;
    philosophers[i].right_fork = &forks[i];
    philosophers[i].left_fork = &forks[(i-1) % PHILOSOPHERS];
    
    int code;
    code = pthread_create(&child_thread[i], NULL, (void*)child, (void*)&philosophers[i]);
    if (code) {
      fprintf(stderr, "pthread_create failed with code %d\n", code);
    }
  } 

  unsigned long thread_consumed;
  unsigned long reported_total_consumed = 0;

  for (unsigned int i = 0; i < PHILOSOPHERS; i++) {
    pthread_join(child_thread[i], (void*)&thread_consumed);
    assert(thread_consumed == HUNGER);
    assert(philosophers[i].food_consumed == HUNGER);
    reported_total_consumed += thread_consumed;
  }

  assert(reported_total_consumed == HUNGER * PHILOSOPHERS);

  printf("Success. All tests passed.\n");
  
  return 0;
}