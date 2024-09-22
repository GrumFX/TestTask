#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <time.h>
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
    static const uint64_t cube_ax_max_size = 15;
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

void gaussianElimination(std::vector<std::vector<uint64_t>>& matrix, std::vector<uint64_t>& solution)
{
    int n = matrix.size();    // Number of equations
    int m = matrix[0].size(); // Number of variables

    auto start = std::chrono::high_resolution_clock::now();

    // Perform Gaussian elimination(mod 2)
    for (int col = 0; col < m; ++col)
    {
        int pivot = -1;



        // Find a row with a leading 1 in this column
        for (int row = col; row < n; ++row)
        {
            if (matrix[row][col] == 1)
            {
                pivot = row;
                break;
            }
        }

        if (pivot == -1)
        {
            // No pivot found, column is already "solved"
            continue;
        }

        // Swap pivot row with current row
        std::swap(matrix[pivot], matrix[col]);
#pragma omp parallel for
        // Eliminate all other 1's in this column
        for (int row = 0; row < n; ++row)
        {
            if (row != col && matrix[row][col] == 1)
            {
                // XOR the current row with the pivot row to eliminate the 1
                for (int k = col; k < m; ++k)
                {
                    matrix[row][k] ^= matrix[col][k]; // Perform XOR operation
                }
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Time taken For1: " << elapsed.count() << " milliseconds\n";
    // Extract the solution from the reduced matrix
    for (int row = 0; row < n; ++row)
    {
        solution[row] = matrix[row][m - 1]; // Last column contains the solution
    }
}

// Unlock function for solving the cube using XOR logic
void unlock(LockCube& cube)
{
    auto state = cube.read();
    uint64_t x_size = state.size();
    uint64_t y_size = state[0].size();
    uint64_t z_size = state[0][0].size();

    // Number of cells in the cube
    int numCells = x_size * y_size * z_size;

    // Matrix to represent the cube and the click operations
    std::vector<std::vector<uint64_t>> matrix(numCells, std::vector<uint64_t>(numCells + 1, 0)); // Extra column for RHS
    std::vector<uint64_t> solution(numCells, 0);

    // Fill the matrix with the toggling rules (click propagation) and the current cube state
    for (uint64_t x = 0; x < x_size; ++x)
    {
        for (uint64_t y = 0; y < y_size; ++y)
        {
            for (uint64_t z = 0; z < z_size; ++z)
            {
                int cellIndex = x * y_size * z_size + y * z_size + z;

                // Set up the row in the matrix representing the effect of clicking this cell
                matrix[cellIndex][cellIndex] = 1; // Clicking itself

                // Affecting the cells in the same row, column, and depth
                for (uint64_t xi = 0; xi < x_size; ++xi)
                {
                    if (xi != x)
                    {
                        int rowCellIndex = xi * y_size * z_size + y * z_size + z;
                        matrix[cellIndex][rowCellIndex] = 1; // Affect the cell in the same row
                    }
                }

                for (uint64_t yi = 0; yi < y_size; ++yi)
                {
                    if (yi != y)
                    {
                        int colCellIndex = x * y_size * z_size + yi * z_size + z;
                        matrix[cellIndex][colCellIndex] = 1; // Affect the cell in the same column
                    }
                }

                for (uint64_t zi = 0; zi < z_size; ++zi)
                {
                    if (zi != z)
                    {
                        int depthCellIndex = x * y_size * z_size + y * z_size + zi;
                        matrix[cellIndex][depthCellIndex] = 1; // Affect the cell in the same depth
                    }
                }

                // Set the right-hand side to the current cube state (locked or unlocked)
                matrix[cellIndex][numCells] = state[x][y][z];
            }
        }
    }

    // Perform Gaussian elimination (mod 2) to find the solution
    gaussianElimination(matrix, solution);

    // Apply the clicks found in the solution to the cube
    for (uint64_t x = 0; x < x_size; ++x)
    {
        for (uint64_t y = 0; y < y_size; ++y)
        {
            for (uint64_t z = 0; z < z_size; ++z)
            {
                int cellIndex = x * y_size * z_size + y * z_size + z;

                // If the solution for this cell is 1, we need to click it
                if (solution[cellIndex] == 1)
                {
                    cube.click(x, y, z);
                }
            }
        }
    }

    std::cout << "Cube unlocked using XOR logic.\n";
}

int main()
{
    auto startcube = std::chrono::high_resolution_clock::now();
    LockCube cube;
    auto endcube = std::chrono::high_resolution_clock::now();
    auto elapsedcube = std::chrono::duration_cast<std::chrono::milliseconds>(endcube - startcube);

    std::cout << "Time taken to create the cube: " << elapsedcube.count() << " milliseconds\n";
    auto sizeOfCube = cube.read();
    std::cout << "Size: " << sizeOfCube.size() << "\n";

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

    auto state = cube.read();

    uint64_t x_size = state.size();
    uint64_t y_size = state[0].size();
    uint64_t z_size = state[0][0].size();

    // Iterate through every element in the 3D array of the cube
    /*for (uint64_t x = 0; x < x_size; x++)
    {
        std::cout << std::endl;
        for (uint64_t y = 0; y < y_size; y++)
        {
            std::cout << std::endl;
            for (uint64_t z = 0; z < z_size; z++)
            {
                std::cout << state[x][y][z] << " ";

            }
        }
    }*/
    return 0;
}