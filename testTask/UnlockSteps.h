#pragma once
#include <vector>
#include <iostream>
#include <omp.h>
#include "BitRow.h"


inline uint64_t coordToIndex(uint64_t x, uint64_t y, uint64_t z, uint64_t y_size, uint64_t z_size)
{
    return x * y_size * z_size + y * z_size + z;
}

std::vector<BitRow> constructMatrix(const std::vector<std::vector<std::vector<bool>>>& state, uint64_t cellCount, uint64_t x_size, uint64_t y_size, uint64_t z_size);
size_t gaussianElimination(std::vector<BitRow>& augmented, uint64_t cellCount, std::vector<size_t>& pivot_cols);
std::vector<bool> backSubstitution(const std::vector<BitRow>& augmented, const std::vector<size_t>& pivot_cols, size_t rank, uint64_t N);
