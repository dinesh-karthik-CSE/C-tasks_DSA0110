#include <iostream>
#include <memory>
#include <vector>

class Account {
public:
    virtual double rate() const { return 3.5; }
    virtual ~Account() {
        std::cout << "[Dtor] ~Account() called\n";
    }
};

class SeniorAccount : public Account {
    double* history = new double[100];
public:
    double rate() const override { return 4.0; }
    ~SeniorAccount() override {
        std::cout << "[Dtor] ~SeniorAccount() called (freeing history cleanly)\n";
        delete[] history;
    }
};

int main() {
    std::cout << "--- Executing Repaired Polymorphic Account Code ---\n";
    std::vector<std::unique_ptr<Account>> accounts;
    accounts.push_back(std::make_unique<Account>());
    accounts.push_back(std::make_unique<SeniorAccount>());

    for (const auto& acc : accounts) {
        std::cout << "Rate: " << acc->rate() << "%\n";
    }
    std::cout << "--- Leaving scope (automatic virtual destruction) ---\n";
    return 0;
}
