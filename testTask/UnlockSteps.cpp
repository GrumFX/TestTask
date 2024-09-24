#include "UnlockSteps.h"

std::vector<BitRow> constructMatrix(const std::vector<std::vector<std::vector<bool>>>& state, uint64_t cellCount, uint64_t x_size, uint64_t y_size, uint64_t z_size)
{
    std::vector<BitRow> augmentedMatrix(cellCount, BitRow(cellCount + 1));

    for (uint64_t x = 0; x < x_size; x++)
    {
        for (uint64_t y = 0; y < y_size; y++)
        {
            for (uint64_t z = 0; z < z_size; z++)
            {
                uint64_t row_idx = coordToIndex(x, y, z, y_size, z_size);

                for (uint64_t i = 0; i < x_size; i++)
                {
                    uint64_t var_idx = coordToIndex(i, y, z, y_size, z_size);
                    augmentedMatrix[row_idx].setBit(var_idx);
                }
                for (uint64_t j = 0; j < y_size; j++)
                {
                    uint64_t var_idx = coordToIndex(x, j, z, y_size, z_size);
                    augmentedMatrix[row_idx].setBit(var_idx);
                }
                for (uint64_t k = 0; k < z_size; k++)
                {
                    uint64_t var_idx = coordToIndex(x, y, k, y_size, z_size);
                    augmentedMatrix[row_idx].setBit(var_idx);
                }

                if (state[x][y][z])
                {
                    augmentedMatrix[row_idx].setBit(cellCount, true);
                }
            }
        }
    }

    return augmentedMatrix;
}

size_t gaussianElimination(std::vector<BitRow>& augmented, uint64_t cellCount, std::vector<size_t>& pivot_cols)
{
    size_t rank = 0;

    for (size_t col = 0; col < cellCount && rank < cellCount; col++)
    {
        if (col % 1000 == 0)
        {
            std::cout << "Col: " << col << " from: " << cellCount << std::endl;
        }

        size_t sel = rank;
        while (sel < cellCount && !augmented[sel].getBit(col))
        {
            sel++;
        }

        if (sel == cellCount)
        {
            continue;
        }

        if (sel != rank)
        {
            std::swap(augmented[sel], augmented[rank]);
        }

        pivot_cols.push_back(col);

#pragma omp parallel for schedule(dynamic)
        for (long long row = 0; row < cellCount; row++)
        {
            if (row != rank && augmented[row].getBit(col))
            {
#pragma omp critical
                {
                    augmented[row].xorWith(augmented[rank]);
                }
            }
        }
        rank++;
    }

    return rank;
}

std::vector<bool> backSubstitution(const std::vector<BitRow>& augmented, const std::vector<size_t>& pivot_cols, size_t rank, uint64_t N)
{
    std::vector<bool> clicks(N, false);

    for (int i = rank - 1; i >= 0; i--)
    {
        size_t col = pivot_cols[i];
        bool val = augmented[i].getBit(N);
        for (size_t j = col + 1; j < N; j++)
        {
            if (augmented[i].getBit(j))
            {
                val ^= clicks[j];
            }
        }
        clicks[col] = val;
    }

    return clicks;
}