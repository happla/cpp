//
// Created by Hannu Pham on 22.8.2025.
//
// whileloop with a decrement operator from 10 to 0

#include <iostream>

int main() {
    int sum = 0, val = 10;

    while (val <= 0) {
        sum -= val;
        --val; //decrement 1 to val
    }
    std::cout << sum << std::endl;
    return 0;
}