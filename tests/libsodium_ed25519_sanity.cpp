#include <sodium.h>
#include <array>
#include <string_view>

int main() {
    if (sodium_init() < 0) {
        return 1; // Initialization failed
    }

    const std::string_view msg = "phoenix-libsodium-smoketest";
    std::array<unsigned char, crypto_sign_PUBLICKEYBYTES> pk{};
    std::array<unsigned char, crypto_sign_SECRETKEYBYTES> sk{};
    std::array<unsigned char, crypto_sign_BYTES + 64> sigbuf{}; // msg size = 64 bytes or less
    unsigned long long siglen = 0;

    if (crypto_sign_keypair(pk.data(), sk.data()) != 0) {
        return 2;
    }

    if (crypto_sign_detached(sigbuf.data(), &siglen,
                             reinterpret_cast<const unsigned char*>(msg.data()),
                             msg.size(),
                             sk.data()) != 0) {
        return 3;
    }

    if (crypto_sign_verify_detached(sigbuf.data(),
                                    reinterpret_cast<const unsigned char*>(msg.data()),
                                    msg.size(),
                                    pk.data()) != 0) {
        return 4; // signature invalid (should not happen)
    }

    return 0;
}

