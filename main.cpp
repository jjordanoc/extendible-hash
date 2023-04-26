#include <functional>
#include <iostream>
#include <sstream>

#include "ExtendibleHashFile.hpp"

void readFromConsole(char buffer[], int size) {
    std::string temp;
    std::cin >> temp;
    for (int i = 0; i < size; i++)
        buffer[i] = (i < temp.size()) ? temp[i] : ' ';
    buffer[size - 1] = '\0';
    std::cin.clear();
}

struct Record {
    char code[5];
    char name[20];
    int cycle;

    std::string to_string() {
        std::stringstream ss;
        ss << "(" << code << ", " << name << ", " << cycle << ")";
        return ss.str();
    }
};


int main() {
    std::function<bool(int, int)> equal = [](int a, int b) {
        return a == b;
    };

    std::function<int(Record &)> index = [=](Record &record) {
        return record.cycle;
    };
    std::hash<int> hasher;
    std::function<std::size_t(int)> hash = [&hasher](int key) {
        return hasher(key);
    };

    ExtendibleHashFile<int, Record, decltype(equal), decltype(index), decltype(hash), 3> extendibleHash{"data.dat", false, index, equal, hash};
//    Record new_record{};
//    readFromConsole(new_record.code, 5);
//    readFromConsole(new_record.name, 20);
//    std::cin >> new_record.cycle;
//    extendibleHash.insert(new_record);
    extendibleHash.remove(80);
    auto result = extendibleHash.search(80);
    for (auto &record: result) {
        std::cout << record.to_string() << std::endl;
    }

    return 0;
}
