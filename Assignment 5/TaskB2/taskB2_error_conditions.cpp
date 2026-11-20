#include <iostream>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

class HospitalException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class BillingValidationError : public HospitalException {
public:
    using HospitalException::HospitalException;
};

class ClaimError : public HospitalException {
public:
    using HospitalException::HospitalException;
};

class ClaimLimitExceededError : public ClaimError {
public:
    using ClaimError::ClaimError;
};

class UnknownPackageError : public ClaimError {
public:
    using ClaimError::ClaimError;
};

struct TransactionGuard {
    std::string label;
    explicit TransactionGuard(std::string l) : label(std::move(l)) {
        std::cout << "  [RAII Acquire] " << label << "\n";
    }
    ~TransactionGuard() {
        std::cout << "  [RAII Release] ~" << label << " destroyed cleanly\n";
    }
};

double parseAndValidateCharge(const std::string& rawInput) {
    TransactionGuard guard("ChargeParser(" + rawInput + ")");
    size_t idx = 0;
    double val = std::stod(rawInput, &idx); // Throws std::invalid_argument on non-numeric text
    if (idx != rawInput.size()) {
        throw BillingValidationError("Trailing garbage in charge input: " + rawInput);
    }
    if (val < 0.0) {
        throw BillingValidationError("Negative charge not allowed: " + std::to_string(val));
    }
    return val;
}

double settleInsuranceClaim(double amount, double coverLimit, const std::string& pkgCode) {
    TransactionGuard guard("ClaimSession(" + pkgCode + ")");
    static const std::unordered_map<std::string, double> validPackages = {
        {"OP-STD", 2000.0}, {"IP-GEN", 50000.0}, {"CATARACT", 25000.0}
    };

    auto it = validPackages.find(pkgCode);
    if (it == validPackages.end()) {
        throw UnknownPackageError("Unknown health scheme package code: " + pkgCode);
    }
    if (amount > coverLimit || amount > it->second) {
        throw ClaimLimitExceededError("Claim Rs." + std::to_string(amount) +
                                      " exceeds allowed limit");
    }
    return amount;
}

void triggerControlledBadAlloc() {
    TransactionGuard guard("HeapBufferAlloc");
    // Requesting an impossibly large block (~8 Exabytes) to trigger std::bad_alloc
    volatile size_t hugeCount = static_cast<size_t>(-1) / sizeof(double);
    double* ptr = new double[hugeCount];
    delete[] ptr;
}

int main() {
    std::cout << "=== TASK B2: ENGINEERED ERROR CONDITIONS & RAII UNWINDING ===\n";

    // Case 1: Negative charge
    try {
        std::cout << "\n[Test 1] Negative charge (-450):\n";
        parseAndValidateCharge("-450");
    } catch (const HospitalException& e) {
        std::cout << "  Caught HospitalException: " << e.what() << "\n";
    }

    // Case 2: Claim above cover
    try {
        std::cout << "\n[Test 2] Claim above cover (65000 > 50000):\n";
        settleInsuranceClaim(65000.0, 50000.0, "IP-GEN");
    } catch (const ClaimError& e) {
        std::cout << "  Caught ClaimError: " << e.what() << "\n";
    }

    // Case 3: Unknown package code
    try {
        std::cout << "\n[Test 3] Unknown package code (CARDIO-X9):\n";
        settleInsuranceClaim(15000.0, 50000.0, "CARDIO-X9");
    } catch (const ClaimError& e) {
        std::cout << "  Caught ClaimError: " << e.what() << "\n";
    }

    // Case 4: Non-numeric text passed to std::stod
    try {
        std::cout << "\n[Test 4] Text where number is expected (\"ABC_FEE\"):\n";
        parseAndValidateCharge("ABC_FEE");
    } catch (const std::invalid_argument& e) {
        std::cout << "  Caught std::invalid_argument from std::stod: " << e.what() << "\n";
    }

    // Case 5: Memory allocation failure (std::bad_alloc)
    try {
        std::cout << "\n[Test 5] Memory allocation failure (std::bad_alloc):\n";
        triggerControlledBadAlloc();
    } catch (const std::bad_alloc& e) {
        std::cout << "  Caught std::bad_alloc: " << e.what() << "\n";
    }

    // Case 6: Unknown non-std exception caught by top-level catch(...)
    try {
        std::cout << "\n[Test 6] Top-level catch(...) safety net:\n";
        TransactionGuard outerGuard("BatchSession");
        throw 999; // Non-std::exception error
    } catch (const std::exception& e) {
        std::cout << "  Caught std::exception: " << e.what() << "\n";
    } catch (...) {
        std::cout << "  Caught unknown error in catch(...) — system stays alive!\n";
    }

    std::cout << "\n[Recovery Complete] All 6 error scenarios survived with zero leaks.\n";
    return 0;
}
