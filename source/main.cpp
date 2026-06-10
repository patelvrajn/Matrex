#include <iostream>

#include "tuner.hpp"
#include "uci.hpp"

/*******************************************************************************
    Only the entry point (main) function should be in this file because this
    file is excluded from testing.
*******************************************************************************/

int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        UCI uci;
        uci.loop();
    }
    else
    {
        if (std::string(argv[1]) == "tune")
        {
            std::ofstream log_file("assets/tuner.log");
            std::ifstream dataset_file("assets/lichess-big3-resolved.book");
            std::ofstream output_file("assets/evaluation_terms.hpp");
            Tuner         tuner(log_file, dataset_file, output_file);
            tuner.tune();
        }
        else
        {
            std::cerr << "Invalid argument." << std::endl;
        }
    }

    return 0;
}
