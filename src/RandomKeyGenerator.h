#ifndef RANDOM_KEY_GENERATOR_H
#define RANDOM_KEY_GENERATOR_H

#include "secp256k1.h"
#include <random>
#include <cstdint>

class RandomKeyGenerator {
public:
    RandomKeyGenerator();
    ~RandomKeyGenerator() = default;

    secp256k1::uint256 generateRandom256();
    secp256k1::uint256 generateRandom256ForGPU(int gpuId, int totalGPUs);

private:
    std::random_device rd_;
    std::mt19937_64 gen_;
    std::uniform_int_distribution<uint64_t> dist_;
};

#endif // RANDOM_KEY_GENERATOR_H

