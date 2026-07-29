#ifndef FD_CRYPTO_H
#define FD_CRYPTO_H

#define FD_CRYPTO_STACK_SIZE    1024u
#define FD_CRYPTO_REGION_SIZE   2048u

void FDCryptoClear(void);

void FDCryptoTask(void * arg);

#endif
