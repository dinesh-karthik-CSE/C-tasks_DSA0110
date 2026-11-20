#include <iostream>
struct Bill {
    ~Bill() { std::cout << "~Bill "; } // Non-virtual destructor
    virtual double total() const = 0;
};
struct InPatientLeak : Bill {
    double* ledger = new double[50];
    ~InPatientLeak() { std::cout << "~InPatientLeak "; delete[] ledger; }
    double total() const override { return 47500.0; }
};
int main() {
    Bill* b = new InPatientLeak();
    std::cout << "Total: " << b->total() << "\n";
    delete b;
    std::cout << "\n";
    return 0;
}
