#ifndef FD_CRYPTO_H
#define FD_CRYPTO_H

#define FD_CRYPTO_REGION_SIZE       2048u
#define FD_CRYPTO_STACK_WORDS       512u
#define FD_CRYPTO_SUBREGION_SIZE    256u

void FDCryptoClear(void);

void FDCryptoTaskInit(void);

#endif
