#ifndef _ENCRYPTOPS_H
#define _ENCRYPTOPS_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include <cryptlib.h>
#include <aes.h>
#include <modes.h>
#include <osrng.h>

class EncryptOps
{
private:
    std::unordered_map<std::string, std::vector<uint8_t>> map;
    const size_t AES_SIZE = 256 / 8;
    const size_t IV_SIZE = CryptoPP::AES::BLOCKSIZE;

private:
public:
    const std::unordered_map<std::string, std::vector<uint8_t>> *const get_immutable_map();
    std::unordered_map<std::string, std::vector<uint8_t>> *const get_mutable_map();

    void generate_key_iv();
    void encrypt_api_key(const std::string &api_key);
    std::string decrypt_api_key();

    void clear_map();

    const size_t get_aes_size() const;
    const size_t get_iv_size() const;
};

#endif