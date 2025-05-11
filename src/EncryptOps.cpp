#include "EncryptOps.h"

const std::unordered_map<std::string, std::vector<uint8_t>> *const EncryptOps::get_immutable_map()
{
    return &map;
}

std::unordered_map<std::string, std::vector<uint8_t>> *const EncryptOps::get_mutable_map()
{
    return &map;
}

void EncryptOps::clear_map()
{
    map.clear();
}

void EncryptOps::generate_key_iv()
{
    std::vector<uint8_t> key(AES_SIZE);
    std::vector<uint8_t> iv(IV_SIZE);
    CryptoPP::BlockingRng rand;
    rand.GenerateBlock(key.data(), key.size());
    rand.GenerateBlock(iv.data(), iv.size());

    map["key"] = key;
    map["iv"] = iv;
}

void EncryptOps::encrypt_api_key(const std::string &api_key)
{
    std::string cypher;
    auto aes = CryptoPP::AES::Encryption(map.at("key").data(), map.at("key").size());
    auto aes_cbc = CryptoPP::CBC_Mode_ExternalCipher::Encryption(aes, map.at("iv").data());
    CryptoPP::StringSource(
        api_key,
        true,
        new CryptoPP::StreamTransformationFilter(
            aes_cbc,
            new CryptoPP::StringSink(cypher)));

    // had to do this so i can put in the map;
    std::vector<uint8_t> api_cypher;
    for (auto c : cypher)
    {
        api_cypher.push_back(static_cast<uint8_t>(c));
    }
    map["encrypted_apikey"] = api_cypher;
}

const size_t EncryptOps::get_aes_size() const
{
    return AES_SIZE;
}
const size_t EncryptOps::get_iv_size() const
{
    return IV_SIZE;
}

std::string EncryptOps::decrypt_api_key()
{
    std::string decrypted_str;
    std::string cypher(map.at("encrypted_apikey").begin(), map.at("encrypted_apikey").end());
    auto aes = CryptoPP::AES::Decryption(map.at("key").data(), map.at("key").size());
    auto aes_cbc = CryptoPP::CBC_Mode_ExternalCipher::Decryption(aes, map.at("iv").data());
    CryptoPP::StringSource ss(
        cypher,
        true,
        new CryptoPP::StreamTransformationFilter(
            aes_cbc,
            new CryptoPP::StringSink(decrypted_str)));
    return decrypted_str;
}
