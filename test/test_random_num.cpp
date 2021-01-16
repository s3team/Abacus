#include <random>
#include <iostream>
#include "MonteCarlo.hpp"

int main()
{
    std::random_device random_device; // create object for seeding
    std::mt19937 engine{random_device()}; // create engine and seed it
    std::uniform_int_distribution<> dist(0,255); // create distribution for integers with [1; 9] range
    auto random_number = dist(engine); // finally get a pseudo-randomrandom integer number

    std::cout << random_number << std::endl;

    auto randVector = tana::MonteCarlo::getRandomVector(10);

    for (const auto r : randVector)
    {
        std:: cout << +r << " ";
    }
    std::cout << std::endl;
}