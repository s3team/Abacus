#include <stdint.h>
#include <stdio.h>

// We use this empty function to annotate the address and the length of the
// secret
int __attribute__((optimize(0)))
abacus_make_symbolic(char *name, void *addr, uint32_t length) {
  return 1;
}

void is_odd(uint16_t secret) {
  int res = secret % 2;        
  if (res) {             
    printf("Odd Number\n");
  } else {
    printf("Even Number\n");
  }
}

int main() {
  uint16_t secret = 6;
  // uint8_t secret = 8;
  char *type = "1";
  abacus_make_symbolic(type, &secret, 2);   // The length of the key is two bytes
  is_odd(secret);
  return 0;
}
