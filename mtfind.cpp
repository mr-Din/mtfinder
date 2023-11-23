#include <Windows.h>
#include <iostream>

#include "MyMtFinder.h"
#include "MtFinderBlocks.h"
#include "MtFinderReadToVector.h"

int main(int argc, char** argv) {

    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    if (argc < 3) {
        std::cerr << "Usage: mtfind <filename> <mask>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::string mask = argv[2];
        
    MyMtFinder finder(filename, mask);
    finder.RunSearch();
    //finder.Print();

    MtFinderReadToVector finder2(filename, mask);
    finder2.RunSearch();
    finder2.Print();


    // NOT CORRECT
    MtFinderBlocks finder3(filename, mask);
    //finder3.RunSearch();
    //finder3.Print();

    return 0;
}
