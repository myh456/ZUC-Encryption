#include <iostream>
#include <string>

#include "zuc.hpp"

// 输出一个流密钥
void outputKey(u32 key) {
    cout << "0x";
    for (int i = 0; i < 8; i++) {
        cout << hex << (key >> (28 - i * 4) & 0xf);
    }
}

int main(int argc, char const* argv[]) {
    u128 key, iv;
    int keylen;
    string K, IV;
    cout << "输入密钥(32位十六进制数):" << endl;
    cin >> K;
    cout << "输入初始向量(32位十六进制数):" << endl;
    cin >> IV;
    cout << "输入所需流密钥数" << endl;
    cin >> keylen;
    for (int i = 0; i < 16; i++) {
        key[i] = iv[i] = 0;
        key[i] |= K[i * 2];
        key[i] <<= 4;
        key[i] |= K[i * 2 + 1];
        iv[i] |= K[i * 2];
        iv[i] <<= 4;
        iv[i] |= K[i * 2 + 1];
    }
    ZUC zuc = ZUC(key, iv);
    for (int i = 0; i < keylen; i++) {
        cout << "第" << i << "个:";
        outputKey(zuc.subkey());
        cout << endl;
    }
    return 0;
}
