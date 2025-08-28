//
// Created by Hannu Pham on 27.8.2025.
//collect dirnames, store in vector, form a relative path,
//goal is to train for avoiding pollution

#include <iostream>
#include <vector>
#include <string>
#include <filesystem> // for path and dir functions
#include <fstream> // file writing


//pollution avoidance pull out necessary declarations with "using" instead of using namespace std;
using std::cout;
using std::cin;
using std::vector;
using std::string;
using std::getline;
using std::ofstream;
using std::filesystem::path;
using std::filesystem::create_directories;
using std::filesystem::absolute;


int main() {
    //read dir names from user
    vector<string> dirNames;
    string input;

while (true) {
    cout << "Enter path to directory names or empty line to stop: \n";
    getline(cin,input);
    if (input.empty()) break;
    dirNames.push_back(input);
}

    /*
    cout << "Enter path to directory names or empty line to stop: ";
    getline(cin, input); // read first line
    while (!input.empty()) { // continue until blank
        dirNames.push_back(input); //store in vector
        cout << "Enter path to directory names or empty line to stop: ";
        getline(cin, input); //read next line
    }
*/
    path dirPath;
    for (const auto& name : dirNames) {
        dirPath /=name; //append dir to path
    }

    create_directories(dirPath); //create dir if needed

    path filePath = dirPath / "result.txt"; // point to the file
    ofstream outFile(filePath); //handles writing

    if (!outFile) {
        cout << "failed to open file!\n";
        return 1;
    }

    path absolutePath = absolute(filePath); //convert to full system path:linux or windows

    cout << absolutePath << "\n"; //print path for user
    outFile << absolutePath << "\n"; //writes result.txt file

    for (const auto& name : dirNames) { // loops through each name in the vector
        cout << name << "\n"; //prints name to console
        outFile << name << "\n";  //write to file

    }
    cout << filePath << "\n"; //print how the path looks
    outFile << filePath << "\n"; // writing to keep record of relative path

}
