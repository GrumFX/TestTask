#include "BitRow.h"

BitRow::BitRow(size_t num_bits)
{
    _bits.resize((num_bits + 63) / 64, 0);
}

void BitRow::setBit(size_t pos)
{
    _bits[pos / 64] |= (1ULL << (pos % 64));
}

void BitRow::setBit(size_t pos, bool value)
{
    if (value)
        _bits[pos / 64] |= (1ULL << (pos % 64));
    else
        _bits[pos / 64] &= ~(1ULL << (pos % 64));
}

bool BitRow::getBit(size_t pos) const
{
    return (_bits[pos / 64] >> (pos % 64)) & 1ULL;
}

void BitRow::xorWith(const BitRow& other)
{
    for (size_t i = 0; i < _bits.size(); ++i)
    {
        _bits[i] ^= other._bits[i];
    }
}
