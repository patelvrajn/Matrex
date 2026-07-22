#pragma once

#include <cstdint>

// =============================================================================
// xorshift64star Pseudo-Random Number Generator
// This class is based on original code written and dedicated to the public
// domain by Sebastiano Vigna (2014).
//
// It has the following characteristics:
//
//  -  Outputs 64-bit numbers
//  -  Passes Dieharder and SmallCrush test batteries
//  -  Does not require warm-up, no zeroland to escape
//  -  Internal state is a single 64-bit integer
//  -  Period is 2^64 - 1
//
// For further analysis see
//   <http://vigna.di.unimi.it/ftp/papers/xorshift.pdf>
// =============================================================================
constexpr uint64_t GLOBAL_PRNG_DEFAULT_SEED = 1070372; // Seed from Stockfish

template <typename T>
class Psuedo_RNG
{

  public:

    constexpr Psuedo_RNG(const uint64_t seed = GLOBAL_PRNG_DEFAULT_SEED);

    constexpr T generate_random();
    constexpr T generate_sparse_random();

  private:

    uint64_t m_seed;

    constexpr uint64_t generate_random_64();
};

template <typename T>
constexpr Psuedo_RNG<T>::Psuedo_RNG(const uint64_t seed) : m_seed(seed)
{
}

template <typename T>
constexpr uint64_t Psuedo_RNG<T>::generate_random_64()
{
    m_seed ^= m_seed >> 12;
    m_seed ^= m_seed << 25;
    m_seed ^= m_seed >> 27;

    return m_seed * 2685821657736338717LL;
}

template <typename T>
constexpr T Psuedo_RNG<T>::generate_random()
{
    return static_cast<T>(generate_random_64());
}

template <typename T>
constexpr T Psuedo_RNG<T>::generate_sparse_random()
{
    return static_cast<T>(generate_random_64() & generate_random_64()
                          & generate_random_64());
}
