//
// Created by Hannu Pham on 22.8.2025.
//

//this program uses while loop to sum the numbers from 50 to 100
#include <iostream>

int main () {
    int sum = 0, val = 50;
    while (val <= 100) {
        sum += val;
        ++val;
    }
    std::cout << "sum of numbers from 50 to 100 is " << sum << std::endl;
    return 0;

}