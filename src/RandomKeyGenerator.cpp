#include "RandomKeyGenerator.h"
#include <ctime>

RandomKeyGenerator::RandomKeyGenerator() : gen_(rd_()), dist_(0, UINT64_MAX) {
    gen_.seed(static_cast<unsigned long>(time(nullptr)));
}

secp256k1::uint256 RandomKeyGenerator::generateRandom256() {
    secp256k1::uint256 key;
    // uint256 uses v[8] array of uint32_t, so we generate 8 random 32-bit values
    // Use two 64-bit random values to fill 8 uint32_t slots
    for (int i = 0; i < 4; i++) {
        uint64_t val64 = dist_(gen_);
        key.v[i * 2] = static_cast<uint32_t>(val64);
        key.v[i * 2 + 1] = static_cast<uint32_t>(val64 >> 32);
    }
    return key;
}

secp256k1::uint256 RandomKeyGenerator::generateRandom256ForGPU(int gpuId, int totalGPUs) {
    secp256k1::uint256 key = generateRandom256();
    // Add GPU-specific offset to ensure different starting points
    uint64_t offset = static_cast<uint64_t>(gpuId) * 0x100000000ULL;
    key.v[0] += static_cast<uint32_t>(offset);
    key.v[1] += static_cast<uint32_t>(offset >> 32);
    return key;
}

