//
// Created by Hannu Pham on 21.8.2025.
//

// EX1  Write a program that defines an empty vector of integers. Then program asks user how many
//numbers he/she wants to enter. Then program asks user to enter the numbers one at a time and
//adds them to the vector. When all number have been entered the program prints the numbers in the
//vector.
#include <iostream>

int main() {
  //asks user for how many numbers here
  int n;
  std::cout << "How many numbers do you want to enter?";
  std::cin >> n;

  // empty vector array here to store numbers
  std::vector<int> numbers; // empty vector here
  // loop here until reached the quota for entered numbers.
  for (int i = 0; i < n; i++) {
    int value;
    std::cout << "Enter a nr [" << i + 1 << "]" << ": ";
    std::cin >> value;
    numbers.push_back(value); // store in vector

  }


  // print the numbers from the vector array, (loop to print all)
  std::cout << "You entered: ";
  for (int num : numbers) {
    std::cout << num << " ";
  }
  std::cout << std::endl;
  return 0;
}


/* multiplication operator test below
int main() {
  std::cout << "Enter two numbers: " << std::endl;


  int v1 = 0, v2 = 0;
  std::cin >> v1 >> v2;
    std::cout << "The sum of " << v1 << " and " << v2
    << " is " << v1 * v2 << std::endl;

    return 0;

}
*/
