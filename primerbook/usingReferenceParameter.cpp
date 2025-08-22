//
// Created by Hannu Pham on 21.8.2025.
//
/*
* Write a function that takes a reference to string and a character (by value) as parameters. The
function replaces all occurrences of the character with underscores.
Write a program that ask user to enter a string. Then program asks user to enter character to replace
or “stop” to stop the program. Note that program must read a string after the second question to be
able to test for “stop”. If answer is not “stop” then program passes the first character of second
question’s answer as the character to replace. Then program prints the string and continues. If
answer is “stop” the program ends.
 */
#include <iostream>

//function with reference to a string and character(value), all occurrences with _
void replaceWithUnderscore(std::string &s, char c) {
    for (char &ch : s) {
        if (ch == c) {
            ch = '_';
        }
    }
}

int main() {
    std::string text;

    //ask user to enter a string
    while (true) {
        std::cout << "Enter a string: ";
        getline(std::cin, text);

        std::string inputText;
        std::cout << "Enter a character to replace or 'stop': "; // ask user to enter a character to replace or stop
        getline(std::cin, inputText);
        if (inputText == "stop") {
            // stop the program if "stop"
            break; //instead of break use while statement until it becomes false
        }
        // print results
        char c = inputText[0];
        replaceWithUnderscore(text, c);
        std::cout << "Result: " << text << std::endl;
    }
    return 0;

}
