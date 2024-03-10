// Vector dot product example in sequential

// ./seq_dot_product <size>

#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <size>" << std::endl;
        return 1;
    }
    int size = std::stoi(argv[1]);

    double *v1 = new double[size];
    double *v2 = new double[size];
    for (int i = 0; i < size; i++) {
        v1[i] = 1.0;
        v2[i] = 1.0;
    }

    double dot_product = 0;
    for (int i = 0; i < size; i++) {
        dot_product += v1[i] * v2[i];
    }
    std::cout << "Dot product: " << dot_product << std::endl;

    return 0;
}