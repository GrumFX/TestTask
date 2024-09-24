#pragma once
#include <vector>

class BitRow
{
public:
    BitRow(size_t numBits);

    void setBit(size_t pos);

    void setBit(size_t pos, bool value);

    bool getBit(size_t pos) const;

    void xorWith(const BitRow& other);
private:
    std::vector<uint64_t> _bits;
};
