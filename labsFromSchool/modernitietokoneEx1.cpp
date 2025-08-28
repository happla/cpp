//
// Created by Hannu Pham on 28.8.2025.
//

#include <iostream> //cout cin cerr
#include <vector> // stack

using namespace std; // ran into a problem with "stack" name


vector<int> myStack; // simulated stack
int memory[4]; //M0, M1, M2, M3 ; main memory

void push(int value) { //pushes value to the top
    myStack.push_back(value);
}
void pushMemory (int index) { //pushes value to memory/stack
    myStack.push_back(memory[index]);
}

int pop() { // check if stack is empty if not then print error
    if (myStack.empty()) {
        cerr << "Stack underflow!" << endl;
        return 0;
    }
    int val = myStack.back();
    myStack.pop_back();
    return val;
}

void popToMemory(int index) { //pop value off the stack
    memory[index] = pop();
}

void add() { // pop two top values(a+b) result to the top of the stack
    int a = pop();
    int b = pop();
    push(a + b);
}

void mul() { // pop two top values(a*b) result to the top of the stack
    int a = pop();
    int b = pop();
    push(a * b);
}

void setMemory(int index, int value) { //set memory
    memory[index] = value;
}

int getMemory(int index) { // read back a value from mem
    return memory[index];
}

int main() { // M0 = (m0+m1 * m1) * (m3 + m1 * m1) + m2
    setMemory(0, 2); // M0 = 2
    setMemory(1, 3); // M1 = 3
    setMemory(2, 5); // M2 = 5
    setMemory(3, 7); // M3 = 7

    // M0+m1*m1
    pushMemory(1);   // push M1
    pushMemory(1);   // push M1
    mul();           // stack: [M1*M1]
    pushMemory(0);   // push M0
    add();           // stack: [M0 + M1*M1]

    // save m0+m1*m1 to memory temp m0
    popToMemory(0);

    // (m3 + m1 * m1)
    pushMemory(1); //push M1
    pushMemory(1); //push M1
    mul(); //stack: [M1*M1]
    pushMemory(3); // push M3
    add(); // stack: [M3 + M1*M1]

    // multiply the two
    pushMemory(0); //push m0+m1*m1
    mul(); // stack the two (m0+m1*m1) * (m3 + m1 * m1)

    // add m2
    pushMemory(2);
    add(); // stack (..) + m2

    popToMemory(0); // store final result in m0

    //print final result
    cout << "Result: " << getMemory(0) << endl;
}
