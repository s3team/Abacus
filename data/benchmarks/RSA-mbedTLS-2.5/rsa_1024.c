// From https://github.com/zColdWater/rsa-demo
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  int ret = 0;

  // random data generator
  mbedtls_entropy_context entropy;
  mbedtls_entropy_init(&entropy);

  // randomness with seed
  mbedtls_ctr_drbg_context ctr_drbg;
  char *personalization = "My RSA demo";
  mbedtls_ctr_drbg_init(&ctr_drbg);

  ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                              (const unsigned char *)personalization,
                              strlen(personalization));
  if (ret != 0) {
    // ERROR HANDLING CODE FOR YOUR APP
  }
  mbedtls_ctr_drbg_set_prediction_resistance(&ctr_drbg, MBEDTLS_CTR_DRBG_PR_ON);
  ////////////////////////////////////////////////////////////////////////

  mbedtls_pk_context pk;
  mbedtls_pk_init(&pk);

  unsigned char to_encrypt[] = "Bao Qinkun";
  unsigned char to_decrypt[MBEDTLS_MPI_MAX_SIZE];
  const unsigned char pub_key[] =
      "-----BEGIN PUBLIC KEY-----\r\n"
      "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC9Jc728q/SZskvJc28q7wJjN2l\r\n"
      "rY8kV6cDWTtw/TikPugRdnaCdU/hBfahMRsh0d6ccibV3pZe/Hbug5l1yxyykOSe\r\n"
      "gVJx/qWI2FU8LbLHK7XcSzR1n/CogzhVYIHTu7pxKYt2unvmKB+gThUYduHW68xp\r\n"
      "iba04vDfCw7KvSlTtQIDAQAB\r\n"
      "-----END PUBLIC KEY-----\r\n";

  const unsigned char prv_pwd[] = "password";
  const unsigned char prv_key[] =
      "-----BEGIN RSA PRIVATE KEY-----\r\n"
      "MIICXgIBAAKBgQC9Jc728q/SZskvJc28q7wJjN2lrY8kV6cDWTtw/TikPugRdnaC\r\n"
      "dU/hBfahMRsh0d6ccibV3pZe/Hbug5l1yxyykOSegVJx/qWI2FU8LbLHK7XcSzR1\r\n"
      "n/CogzhVYIHTu7pxKYt2unvmKB+gThUYduHW68xpiba04vDfCw7KvSlTtQIDAQAB\r\n"
      "AoGAYp8jEZmqWR8kyQOCCVzV13juXKNpHj7hoxpUpu4xKVpvcCN/WThHpQGR/av4\r\n"
      "BKND2fifDSZY6z/h1y0gx81WsVBPaLr5fVNgR4CLGH7kjCjigUtjIzJ7UpI9+ZCh\r\n"
      "YWMHyjBs1UL0QkhPZe9h1lQQetvfB0lKNuVVltoySyJQP0ECQQD/vssAPrqGR4G5\r\n"
      "HfgVvhFWWdQgnl6DuOQeRza+LBbM/ZPdiPfhhcpIpM65bFJ1Wigta4aiq64MLOYz\r\n"
      "MlNz0MRxAkEAvVYJAd2hsVZfR0rL8PYLHbeYMhrRIf9gYgfu9a5MM440S/rIB0Gh\r\n"
      "moc7CyC0CqJHYKpL8RQHx13KeJKf/fYVhQJBAPul2oyQLOvKWuwzgBSs5NRqKaA7\r\n"
      "FVdZzCW6/zPboEfvUNtRVlB0XJpkiQHNg8nzf8tJnb5dXjKez5ka8SDqERECQQCu\r\n"
      "dqnEG1qUE1emVMjJ1550mqlWehl9L1m72z2ZCyvSUdXksUhCT3q+7p88aL0eE1yc\r\n"
      "OS/TDDcCwW0BX3KnzGsVAkEAtn+JnKHFtHNBN+GgWSdvibeRTZE10GqomsYTUvWR\r\n"
      "Zu4Zb+zMdf/7HCspxo50dpZN6s3WfGUA0fNqYKi3NCsBPg==\r\n"
      "-----END RSA PRIVATE KEY-----\r\n";

  if ((ret = mbedtls_pk_parse_public_key(&pk, pub_key, sizeof(pub_key))) != 0) {
    printf(" failed\n ! mbedtls_pk_parse_public_keyfile returned -0x%04x\n",
           -ret);
    return -1;
  }

  unsigned char buf[MBEDTLS_MPI_MAX_SIZE];
  size_t olen = 0;

  printf("\nGenerating the encrypted value\n");
  fflush(stdout);

  if ((ret = mbedtls_pk_encrypt(&pk, to_encrypt, sizeof(to_encrypt), buf, &olen,
                                sizeof(buf), mbedtls_ctr_drbg_random,
                                &ctr_drbg)) != 0) {
    printf(" failed\n ! mbedtls_pk_encrypt returned -0x%04x\n", -ret);
    return -1;
  }

  for (int idx = 0; idx < strlen(buf); printf("%02x", buf[idx++]))
    ;
  printf("\n");
  mbedtls_pk_context pk1;
  mbedtls_pk_init(&pk1);

  if ((ret = mbedtls_pk_parse_key(&pk1, prv_key, sizeof(prv_key), prv_pwd,
                                  strlen(prv_pwd))) != 0) {
    printf(" failed\n ! mbedtls_pk_parse_keyfile returned -0x%04x\n", -ret);
    return -1;
  }

  unsigned char result[MBEDTLS_MPI_MAX_SIZE];

  printf("\nGenerating the decrypted value");
  fflush(stdout);

  if ((ret = mbedtls_pk_decrypt(&pk1, buf, olen, result, &olen, sizeof(result),
                                mbedtls_ctr_drbg_random, &ctr_drbg)) != 0) {
    printf(" failed\n! mbedtls_pk_decrypt returned -0x%04x\n", -ret);
    return -1;
  } else {
    fflush(stdout);
    printf("\n\n%s----------------\n\n", result);
  }

  return 0;
}