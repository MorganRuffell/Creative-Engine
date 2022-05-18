
#include "pch.h"
#include "CRandomNumberGenerator.h"

namespace CMath
{
    RandomNumberGenerator g_RNG;
}

#ifndef USE_STL

inline uint32_t Twiddle(uint32_t u, uint32_t v)
{
    return (((u & 0x80000000ul) | (v & 0x7FFFFFFFul)) >> 1) ^ ((v & 1ul) * 0x9908B0DFul);
}

uint32_t CMath::RandomNumberGenerator::Rand()
{
    if (m_Pos == n)
    {
        for (uint32_t i = 0; i < n - m; ++i)
            m_State[i] = m_State[i + m] ^ Twiddle(m_State[i], m_State[i + 1]);
        for (uint32_t i = n - m; i < n - 1; ++i)
            m_State[i] = m_State[i + m - n] ^ Twiddle(m_State[i], m_State[i + 1]);
        m_State[n - 1] = m_State[m - 1] ^ Twiddle(m_State[n - 1], m_State[0]);

        m_Pos = 0;
    }

    uint32_t x = m_State[m_Pos++];
    x ^= (x >> 11);
    x ^= (x << 7) & 0x9D2C5680ul;
    x ^= (x << 15) & 0xEFC60000ul;
    return x ^ (x >> 18);
}

void CMath::RandomNumberGenerator::SetSeed(uint32_t s)
{
    m_State[0] = s;

    for (int i = 1; i < n; ++i)
        m_State[i] = 1812433253ul * (m_State[i - 1] ^ (m_State[i - 1] >> 30)) + i;

    m_Pos = n;
}

#endif
