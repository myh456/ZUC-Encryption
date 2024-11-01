#include "zuc.hpp"

// u32的n循环左移pos位
#define LLL(n, pos) ((n) << (pos) | (n) >> (32 - pos))

void ZUC::LFSRInit(u31 u) {
    u31 s16;
    // 1
    u31 v = this->LFSR[0];
    v += this->LFSR[15] << 15 & 0x7fffffff;
    v += this->LFSR[13] << 17 & 0x7fffffff;
    v += this->LFSR[10] << 21 & 0x7fffffff;
    v += this->LFSR[4] << 20 & 0x7fffffff;
    v += this->LFSR[0] << 8 & 0x7fffffff;
    v %= 0x7fffffff;
    // 2
    s16 = (u + v) % 0x7fffffff;
    // 3
    if (!s16) {
        s16 = 0x7ffffff;
    }
    // 4
    for (int i = 0; i < 15; i++) {
        this->LFSR[i] = this->LFSR[i + 1];
    }
    this->LFSR[15] = s16;
}

void ZUC::LFSRWork() {
    // 1
    u31 s16 = this->LFSR[0];
    s16 += this->LFSR[15] << 15 & 0x7fffffff;   // 写的是u31但int是32位的
    s16 += this->LFSR[13] << 17 & 0x7fffffff;
    s16 += this->LFSR[10] << 21 & 0x7fffffff;
    s16 += this->LFSR[4] << 20 & 0x7fffffff;
    s16 += this->LFSR[0] << 8 & 0x7fffffff;
    s16 %= 0x7fffffff;
    // 2
    if (!s16) {
        s16 = 0x7ffffff;
    }
    // 3
    for (int i = 0; i < 15; i++) {
        this->LFSR[i] = this->LFSR[i + 1];
    }
    this->LFSR[15] = s16;
}

void ZUC::BR() {
    this->X0 = this->X1 = this->X2 = this->X3 = 0;

    this->X0 |= this->LFSR[15] >> 7 & 0xffff;
    this->X0 <<= 8;
    this->X0 |= this->LFSR[14] & 0xffff;

    this->X1 |= this->LFSR[11] & 0xffff;
    this->X1 <<= 8;
    this->X1 |= this->LFSR[9] >> 7 & 0xffff;

    this->X2 |= this->LFSR[7] & 0xffff;
    this->X2 <<= 8;
    this->X2 |= this->LFSR[5] >> 7 & 0xffff;

    this->X3 |= this->LFSR[2] & 0xffff;
    this->X3 <<= 8;
    this->X3 |= this->LFSR[0] >> 7 & 0xffff;
}

u32 ZUC::F() {
    u32 W;
    u32 W1 = 0, W2 = 0;
    u32 S(array<array<array<u8, 16>, 16>, 4> S, u32 temp);
    // 1
    W = this->X0 ^ this->R1;
    W += this->R2;    // 用的是unsigned int做u32，超出2^32会溢出而回绕，相当于取模了
    // 2
    W1 = this->R1 + this->X1;
    // 3
    W2 = this->R2 ^ this->X2;
    // 4
    u32 temp1 = 0, temp2 = 0;
    temp1 |= W1 & 0xffff;
    temp1 <<= 8;
    temp1 |= W2 >> 8 & 0xffff;
    temp2 = temp1 ^ LLL(temp1, 2) ^ LLL(temp1, 10) ^ LLL(temp1, 10) ^ LLL(temp1, 24);
    this->R1 = S(this->s, temp2);
    // 5
    temp1 |= W2 & 0xffff;
    temp1 <<= 8;
    temp1 |= W1 >> 8 & 0xffff;
    temp2 = temp1 ^ LLL(temp1, 8) ^ LLL(temp1, 14) ^ LLL(temp1, 22) ^ LLL(temp1, 30);
    this->R2 = S(this->s, temp2);

    return W;
}

u32 S(array<array<array<u8, 16>, 16>, 4> S, u32 temp) {
    u32 res = 0;
    for (int i = 0; i < 4; i++) {
        res |= S[i][temp >> (7 - i * 2) & 0xf][temp >> (6 - i * 2) & 0xf];
        res <<= 8;
    }
    return res;
}

void ZUC::keySet(u128 key, u128 iv) {
    for (int i = 0; i < 16; i++) {
        this->LFSR[i] = 0;
        this->LFSR[i] |= key[i];
        this->LFSR[i] <<= 15;
        this->LFSR[i] |= this->D[i];
        this->LFSR[i] <<= 8;
        this->LFSR[i] |= iv[i];
    }
}

void ZUC::initialization() {
    u32 W;
    for (int i = 0; i < 32; i++) {
        this->BR();
        W = F();
        LFSRInit(W >> 1);
    }
}

u32 ZUC::subkey() {
    u32 Z;
    this->BR();
    Z = this->F() ^ this->X3;
    LFSRWork();
    return Z;
}

ZUC::ZUC() { }
ZUC::ZUC(u128 key, u128 iv) {
    this->keySet(key, iv);
    this->initialization();
}