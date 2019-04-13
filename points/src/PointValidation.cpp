#include "../include/PointValidation.h"
#include "../include/Parser.h"
#include <algorithm>
#include <thread>
#include <vector>
#include <iostream>
#include <cmath>

static void validateResults_Thread(const std::vector<std::pair<Point, Point>>& solution, const std::vector<std::pair<Point, Point>>& result, uint32_t offset, uint32_t size, uint8_t *isCorrect, uint32_t *errorIndex)
{
    size = std::min<uint32_t>(size, static_cast<uint32_t>(result.size()));

    for (uint32_t i = offset; i < size; ++i)
    {
        if (std::find(solution.begin(), solution.end(), result[i]) == solution.end())
        {
            *errorIndex = i;
            *isCorrect = 0;
            return;
        }
    }
}

bool PointValidation::validatePoints(const std::string& solutionFilename, const std::vector<std::pair<Point, Point>>& result)
{
    //reads the solution in
    std::vector<std::pair<Point, Point>> solution = Parser::readSolution(solutionFilename);

    if (result.size() != solution.size())
    {
        std::cout << "The number of point pairs: " << result.size() << " does not match with the number of the solution: " << solution.size() << " ." << std::endl;
        return false;
    }

    const uint32_t THREAD_NUMBER = std::thread::hardware_concurrency();

    std::vector<std::thread> threads(THREAD_NUMBER);
    std::vector<uint8_t> isCorrect(THREAD_NUMBER, 1);
    std::vector<uint32_t> errorIndex(THREAD_NUMBER, 1);

    const uint32_t pointsPerThread = static_cast<uint32_t>(std::ceil(result.size() / float(THREAD_NUMBER)));

    for (uint32_t i = 0; i < THREAD_NUMBER; i++)
    {
        threads[i] = std::thread(&validateResults_Thread, solution, result, i * pointsPerThread, (i + 1) * pointsPerThread, &isCorrect[i], &errorIndex[i]);
    }

    for (uint32_t i = 0; i < THREAD_NUMBER; i++)
    {
        threads[i].join();
    }

    for (uint32_t i = 0; i < THREAD_NUMBER; i++)
    {
        if (isCorrect[i] == 0)
        {
            std::cout << "The solution: " << result[errorIndex[i]].first << result[errorIndex[i]].second << " is not contained in the solution." << std::endl;
            return false;
        }
    }

    return true;
}
