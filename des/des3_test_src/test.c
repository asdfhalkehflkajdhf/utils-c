
#include <stdio.h>
#include "util-des.h"

void debug_data(uint8_t *data, uint32_t data_len) 
{
    int i;
    for (i=0; i<data_len; i++) {
        printf("0x%x ", data[i]);
    }
}

int des3_test(void)
{
    unsigned char key[24], pt[8], ct[8], tmp[8];
    symmetric_key skey;
    int x, err;

    /*
    if ((err = des_test()) != CRYPT_OK) {
        return err;
    }
    */

    for (x = 0; x < 8; x++) {
        pt[x] = x;
    }

    for (x = 0; x < 24; x++) {
        key[x] = x;
    }

    if ((err = des3_setup(key, 24, 0, &skey)) != CRYPT_OK) {
        return err;
    }

    debug_data(pt, sizeof(pt));
    des3_ecb_encrypt(pt, ct, &skey);

    debug_data(ct, sizeof(ct));
    des3_ecb_decrypt(ct, tmp, &skey);

    if (memcmp(pt, tmp, 8) != 0) {
        printf("diff\n");
        return -1;
    }
    printf("same\n");

    return 0;
}

