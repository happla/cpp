#include <iostream>
#include <fstream>

using namespace std;

void print_matrix(int matrix[5][5]);
int count_sum(int matrix[5][5]);

int main(void)
{
  int matrix[5][5];
  int sum;
  ifstream number("matrix.txt");
  if (!number){
        cout << "Failed to open file...";
  }
  else {
    for (int y=0; y<5;y++){
          for (int x=0;x<5;x++){
            number >> matrix[y][x];
          }
    }
    number.close();
    cout << "Matrix:" << endl;
    print_matrix(matrix);
    sum = count_sum(matrix);
    cout << "Sum of elements: " << sum << endl;
  }
}

// Put your code here / cant change the above code how do I print matrix on screen and calculate the sum of the elements and print the sum using functions print_matrix() and count_sum()
void print_matrix(int matrix[5][5]) {
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x< 5; x++) {
            cout << matrix[y][x] << " ";
        }
        cout << endl;
    }
}

int count_sum(int matrix[5][5]) {
    int sum = 0;
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            sum += matrix[y][x];
        }
    }
    return sum;
}
