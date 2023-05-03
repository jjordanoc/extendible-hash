#include <functional>
#include <iostream>
#include <sstream>

using namespace std;

int main() {
    size_t local_depth = 8;
    size_t seq = 1922;
    size_t seq2 = 38;
//    seq = 1874;
//    cout << ((seq >> (local_depth - 1)) & 1) << endl;
    bool eq = true;
    for (std::size_t i = 0; i < local_depth; ++i) {
        if ((seq & 1) != (seq2 & 1)) {
            cout << "false" << endl;
            eq = false;
            break;
        }
        seq >>= 1;
        seq2 >>= 1;
    }
    if (eq) {
        cout << "true" << endl;
    }


    return 0;
}
