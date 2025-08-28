//
// Created by Hannu Pham on 27.8.2025.
//
//Write program that asks user a line that contains integers or "stop" to stop the program
//otherwise program creates a string stream from the user input and reads from the stream.
//program keeps reading and summing integer or reading int fails because of text or other non num char

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;



//function for reading and summing integer
    int readNumbersonly(const string &line, vector<int> &numbers) { //&line, &numbers ref to what user types in + no mod allowed
        stringstream ss(line); //feed strings to read data
        int number;
        numbers.clear(); //check if vector is empty

        while (ss >> number) { // read integers skip invalid text
            numbers.push_back(number);  // allocate number to the end of the vector
        }

        // calculate sum
        int sum = 0; //start with 0
        for (int i  : numbers) //loop i inside vector numbers
            {
            sum += i;
        }
        return sum; // return total after loop finishes

    }

int main() {
        string line; // store user input
        cout << "Enter a line with numbers or \"stop\": \n"; //ask user for input

        getline(cin, line); //read first line
    //something better than break to stop the program, no for loop! = too brainy
    while (line != "stop") {
        vector<int> numbers;
        int sum = readNumbersonly(line, numbers);

        if (!numbers.empty()) {
        cout << "Total of " << numbers.size() << " is "<< sum << "\n"; //.size gives the number of elements in the vector
        } else {
            cout << "Friendly reminder: input only numbers please. :)\n";
        }
        cout << "Enter a line with numbers or \"stop\": \n";
        getline(cin, line); // read next line
    }

        cout << "thank god! it's ending!\n";
        }
