#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <numeric>

//=========================================================

template< typename Type>
struct Summ
{
    Type summ = 0;
    
    Summ(const Type& start_val)
        : summ(start_val) 
        {}

    void operator() (const Type& arg) 
    {
        summ += arg;
    }

    explicit operator Type() const
    {
        return summ;
    }
};

//=========================================================

int main (const int argc, const char** argv)
{
    const int vec[] = {1, 2, 3, 4, 5};
    const size_t size = sizeof(vec) / sizeof(vec[0]);

    {
        // just cycle with iter

        int summ = 0;

        for (size_t iter = 0; iter < size; iter++)
        {
            summ += vec[iter];
        }

        std::cout << "Summ: " << summ << std::endl;
    }

    {
        // modern style

        int summ = 0;

        for (const auto& x : vec)
        {
            summ += x;
        }

        std::cout << "Summ: " << summ << std::endl;
    }

    {
        // std:: for_each<> with functor

        std::cout << "Summ: " << (int) std::for_each(vec, vec + size, Summ(0)) << std::endl;
    }

    {
        // std:: accumulate 

        std::cout << "Summ: " << std::accumulate(vec, vec + size, 0) << std::endl;
    }

    {
        // lambda-functions

        int summ = 0;
        auto lambdasumm = [&summ] (const int& arg) { summ += arg; };
        
        std::for_each(vec, vec + size, lambdasumm);
        std::cout << "Summ: " << summ << std::endl;
    }

    return 0;
}