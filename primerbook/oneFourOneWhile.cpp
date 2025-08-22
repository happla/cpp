//
// Created by Hannu Pham on 22.8.2025.
//

//while statement
#include <iostream>
int main() {
    int sum = 0, val = 1;
    //keep executing while as long as val is less than or equal to 10
    while (val <= 15) {
        sum += val; // assign sum + val to sum
        ++val; // add 1 to val
    }
    std::cout << "Sum of 1 to 15 is inclusive is " << sum << std ::endl;
    return 0;
}
