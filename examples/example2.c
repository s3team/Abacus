#include <stdint.h>
#include <stdio.h>

// We use this empty function to annotate the address and the length of the
// secret
int __attribute__((optimize(0)))
abacus_make_symbolic(char *name, void *addr, uint32_t length) {
  return 1;
}

uint8_t LookUpTable[64] = {1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8,
                            1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8,
                            1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8,
                            1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8};

void example1(uint8_t input) { // Note that while the input key is 16 bits, this
                               // function only receives 8 bit of the key
  uint8_t key = input;
  uint32_t a = key, b;
  b = a >> 8;
  if (b) {
    printf("branch 1\n");
  } else {
    printf("branch 2\n");
  }
}

int example2(uint8_t input) {
  uint8_t key = input;
  uint32_t a = key, b;
  b = a >> 7;
  if (b) {
    printf("branch 1\n");
  } else {
    printf("branch 2\n");
  }
}

// If the input secret is less than 8, then the leakage will be more serious
// than the secret is larger than 8
int example3(uint8_t secret) {
  if (secret < 8) {
    printf("branch 1\n"); // If an attacker observes that the branch 1 is
                          // executed, it means the 5 bits are zero
  } else {
    printf("branch 2\n");
  }
  if (secret < 128) {
    printf("branch 3\n"); // one bit is zero.
  } else {
    printf("branch 4\n");
  }
  return 0;
}

int example4(uint16_t secret) {
  if (secret < 16) {
    printf("branch 1"); // A serious leakage, the top 12 bits are leaked
  }
  return 0;
}

uint64_t example5(uint16_t secret) {
  int index = secret % 64;
  uint64_t res = LookUpTable[index];
  return res;
}

uint64_t example6(uint16_t secret) {
  int res;
  if (secret > 8) {
    res = 0;
  } else {
    res = 1;
  }
  return res;
}

int main() {
  uint16_t secret = 6;
  // uint8_t secret = 8;
  char *type = "1";
  abacus_make_symbolic(type, &secret, 2); // The length of the key is two bytes
  example1(secret);
  example2(secret);
  example3(secret);
  example4(secret);
  uint64_t res = example5(secret);
  res += example6(secret);
  return res;
}
