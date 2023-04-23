#include <iostream>
#include <fstream>

#define PRINT_SEP std::cout << std::string(100, '=') << std::endl;

void print_file(std::ifstream &file, std::size_t limit) {
    std::string buf;
    while (std::getline(file, buf) && (limit > 0)) {
        std::cout << "(" << buf << ")" << ",";
        --limit;
    }
    std::cout << std::endl;
}

int main() {
    std::size_t limit = 10;
    std::ifstream file;
    file.open("data/contentDataPrime.csv");
    PRINT_SEP
    print_file(file, limit);
    PRINT_SEP
    file.close();
    file.open("data/contentDataRegion.csv");
    PRINT_SEP
    print_file(file, limit);
    PRINT_SEP
    file.close();
    file.open("data/contentDataGenre.csv");
    PRINT_SEP
    print_file(file, limit);
    PRINT_SEP
    file.close();
    return 0;
}