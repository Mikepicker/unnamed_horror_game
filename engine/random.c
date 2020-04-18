#include "random.h"

void random_seed() {
  time_t t;
  srand((unsigned) time(&t));
}

float random_range(float min, float max) {
  return min + (rand() / (float)RAND_MAX) * (max - min);
}
