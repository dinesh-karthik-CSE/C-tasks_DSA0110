#include <iostream>

class Account {
public:
    double rate() const { return 3.5; }
    ~Account() { std::cout << "[Dtor] ~Account() called\n"; }
};

class SeniorAccount : public Account {
    double* history = new double[100];
public:
    double rate() const { return 4.0; }
    ~SeniorAccount() {
        std::cout << "[Dtor] ~SeniorAccount() called (freeing history)\n";
        delete[] history;
    }
};

int main() {
    std::cout << "--- Executing Defective Bank Account Code ---\n";
    Account* accounts[2] = { new Account, new SeniorAccount };
    for (Account* a : accounts) {
        std::cout << "Rate: " << a->rate() << "%\n";
    }
    for (Account* a : accounts) {
        delete a; // UB: ~SeniorAccount() is NOT called because ~Account() is non-virtual!
    }

    std::cout << "\n--- Object Slicing Demonstration ---\n";
    Account copy = SeniorAccount(); // SeniorAccount sub-object is sliced off!
    std::cout << "Sliced copy rate: " << copy.rate() << "%\n";
    return 0;
}
