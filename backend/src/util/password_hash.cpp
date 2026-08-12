#include "util/password_hash.h"

#include <argon2.h>
#include <cstring>
#include <fstream>

namespace devpilot::util {
namespace {

// OWASP 推荐参数：m=19MiB, t=2, p=1（内存受限环境用 m=19456）
constexpr uint32_t kMemoryKib = 19456;
constexpr uint32_t kIterations = 2;
constexpr uint32_t kParallelism = 1;
constexpr uint32_t kSaltLen = 16;
constexpr uint32_t kHashLen = 32;

std::string random_salt()
{
    std::string salt(kSaltLen, '\0');
    std::ifstream urandom("/dev/urandom", std::ios::binary);
    urandom.read(salt.data(), static_cast<std::streamsize>(kSaltLen));
    if (!urandom.good())
    {
        throw std::runtime_error("failed to read /dev/urandom");
    }
    return salt;
}

} // namespace

std::string hash_password(const std::string& password)
{
    const auto salt = random_salt();

    const size_t encoded_len = argon2_encodedlen(
        kIterations, kMemoryKib, kParallelism, kSaltLen, kHashLen, Argon2_id);

    std::string encoded(encoded_len, '\0');
    const int rc = argon2id_hash_encoded(
        kIterations, kMemoryKib, kParallelism,
        password.data(), password.size(),
        salt.data(), kSaltLen, kHashLen,
        encoded.data(), encoded_len);
    if (rc != ARGON2_OK)
    {
        throw std::runtime_error(std::string("argon2id_hash_encoded: ") + argon2_error_message(rc));
    }
    encoded.resize(std::strlen(encoded.data()));
    return encoded;
}

bool verify_password(const std::string& password, const std::string& encoded)
{
    if (encoded.empty())
    {
        return false;
    }
    const int rc = argon2id_verify(encoded.c_str(), password.data(), password.size());
    return rc == ARGON2_OK;
}

} // namespace devpilot::util