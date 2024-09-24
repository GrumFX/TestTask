#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <bitset>
#include <unordered_map>
#include <cstdint>
#include <omp.h>

// Your task is to create a program that unlocks a 3D cube.
// The cube is made up of cells that can either be "locked" or "unlocked." 
// When you click on a cell, it changes its state, and all other cells 
// in the same row, column, and depth (along the X, Y, and Z axes) also change.
// The goal is to write a function called unlock() that ensures all cells 
// in the cube are unlocked.
// The cube is fully unlocked when every cell is in the "unlocked" state.
// Here's the fun part: You can't change the LockCube class or its methods, 
// and you also can't tweak the main() function. Your tool is the click() method 
// to get the job done. No sneaky tricks like direct memory access or variable overwriting—
// keep it clean and efficient! The challenge is to unlock the cube with the 
// fewest number of clicks and in the least amount of time.

class LockCube
{
public:

    static const uint64_t cube_ax_min_size = 10;
    static const uint64_t cube_ax_max_size = 100;
    static const uint64_t cube_lock_frq = 3;

    LockCube();

    void click(uint64_t x, uint64_t y, uint64_t z);
    bool isLock();

    std::vector <std::vector<std::vector<bool>>> read();

private:
    std::mt19937_64 rng;
    std::vector < std::vector<std::vector<bool>>> state;
    uint64_t x_size = 0, y_size = 0, z_size = 0;
};


void LockCube::click(uint64_t x, uint64_t y, uint64_t z)
{
    state[x][y][z] = !state[x][y][z];

    for (uint64_t xi = 0; xi < x_size; xi++)
    {
        if (xi == x)
            continue;

        state[xi][y][z] = !state[xi][y][z];
    }

    for (uint64_t yi = 0; yi < y_size; yi++)
    {
        if (yi == y)
            continue;

        state[x][yi][z] = !state[x][yi][z];
    }

    for (uint64_t zi = 0; zi < z_size; zi++)
    {
        if (zi == z)
            continue;

        state[x][y][zi] = !state[x][y][zi];
    }
}


bool LockCube::isLock()
{
    for (uint64_t x = 0; x < x_size; x++)
    {
        for (uint64_t y = 0; y < y_size; y++)
        {
            for (uint64_t z = 0; z < z_size; z++)
            {
                if (state[x][y][z])
                    return true;
            }
        }
    }
    return false;
}


std::vector<std::vector<std::vector<bool>>> LockCube::read()
{
    return state;
}


LockCube::LockCube()
{
    //  allocate
    rng.seed(time(0));
    x_size = (rng() % (cube_ax_max_size - cube_ax_min_size)) + cube_ax_min_size;
    y_size = (rng() % (cube_ax_max_size - cube_ax_min_size)) + cube_ax_min_size;
    z_size = (rng() % (cube_ax_max_size - cube_ax_min_size)) + cube_ax_min_size;

    state.resize(x_size);
    for (uint64_t x = 0; x < x_size; x++)
    {
        state[x].resize(y_size);
        for (uint64_t y = 0; y < y_size; y++)
        {
            state[x][y].resize(z_size);
        }
    }

    //  lock
    for (uint64_t x = 0; x < x_size; x++)
    {
        for (uint64_t y = 0; y < y_size; y++)
        {
            for (uint64_t z = 0; z < z_size; z++)
            {
                if (rng() % cube_lock_frq)
                {
                    click(x, y, z);
                }
            }
        }
    }
}

void unlock(LockCube& cube)
{
    auto state = cube.read();
    uint64_t x_size = state.size();
    if (x_size == 0) return;
    uint64_t y_size = state[0].size();
    if (y_size == 0) return;
    uint64_t z_size = state[0][0].size();
    if (z_size == 0) return;

    uint64_t N = x_size * y_size * z_size;

    std::vector<BitRow> augmented(N, BitRow(N + 1));

    for (uint64_t x = 0; x < x_size; x++)
    {
        for (uint64_t y = 0; y < y_size; y++)
        {
            for (uint64_t z = 0; z < z_size; z++)
            {
                uint64_t row_idx = coord_to_index(x, y, z, y_size, z_size);

                for (uint64_t i = 0; i < x_size; i++)
                {
                    uint64_t var_idx = coord_to_index(i, y, z, y_size, z_size);
                    augmented[row_idx].set_bit(var_idx);
                }
                for (uint64_t j = 0; j < y_size; j++)
                {
                    uint64_t var_idx = coord_to_index(x, j, z, y_size, z_size);
                    augmented[row_idx].set_bit(var_idx);
                }
                for (uint64_t k = 0; k < z_size; k++)
                {
                    uint64_t var_idx = coord_to_index(x, y, k, y_size, z_size);
                    augmented[row_idx].set_bit(var_idx);
                }

                if (state[x][y][z])
                {
                    augmented[row_idx].set_bit(N, true); 
                }
            }
        }
    }

    size_t rank = 0;
    std::vector<size_t> pivot_cols;

    for (size_t col = 0; col < N && rank < N; col++)
    {
        if (col % 1000 == 0)
        {
            std::cout << "Col: " << col << " from: " << N << std::endl;
        }

        size_t sel = rank;
        while (sel < N && !augmented[sel].get_bit(col))
        {
            sel++;
        }

        if (sel == N)
        {
            continue;
        }

        if (sel != rank)
        {
            std::swap(augmented[sel], augmented[rank]);
        }

        pivot_cols.push_back(col);

#pragma omp parallel for schedule(dynamic)
        for (long long row = 0; row < N; row++)
        {
            if (row != rank && augmented[row].get_bit(col))
            {
#pragma omp critical
                {
                    augmented[row].xor_with(augmented[rank]);
                }
            }
        }
        rank++;
    }

    std::vector<bool> clicks(N, false);

    for (int i = rank - 1; i >= 0; i--)
    {
        size_t col = pivot_cols[i];
        bool val = augmented[i].get_bit(N);
        for (size_t j = col + 1; j < N; j++)
        {
            if (augmented[i].get_bit(j))
            {
                val ^= clicks[j];
            }
        }
        clicks[col] = val;
    }

    for (uint64_t idx = 0; idx < N; idx++)
    {
        if (clicks[idx])
        {
            uint64_t x = idx / (y_size * z_size);
            uint64_t rem = idx % (y_size * z_size);
            uint64_t y = rem / z_size;
            uint64_t z = rem % z_size;
            cube.click(x, y, z);
        }
    }
}


int main()
{
    LockCube cube;
    auto state = cube.read();

    std::cout << "Size x: " << state.size() << "\n";
    std::cout << "Size y: " << state[0].size() << "\n";
    std::cout << "Size z: " << state[0][0].size() << "\n";
    auto start = std::chrono::high_resolution_clock::now();
    unlock(cube);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Time taken to unlock the cube: " << elapsed.count() << " milliseconds\n";

    bool result = cube.isLock();
    if (!result)
        std::cout << "The cube is unlocked!\n";
    else
        std::cout << "The cube is still locked!\n";

    return 0;
}