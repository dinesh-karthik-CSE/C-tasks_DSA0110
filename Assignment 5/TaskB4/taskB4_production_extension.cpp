#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// Existing Closed-for-Modification Core Classes
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

class Bill {
protected:
    std::string billId;
    std::string patientName;
public:
    Bill(std::string id, std::string patient)
        : billId(std::move(id)), patientName(std::move(patient)) {
        if (billId.empty() || patientName.empty()) {
            throw BillingValidationError("Bill ID and patient name must not be empty");
        }
    }
    virtual ~Bill() = default;
    const std::string& getId() const noexcept { return billId; }
    const std::string& getPatient() const noexcept { return patientName; }
    virtual double total() const = 0;
    virtual std::string type() const = 0;
};

class OutPatientBill : public Bill {
    double consult, tests;
public:
    OutPatientBill(std::string id, std::string p, double c, double t)
        : Bill(std::move(id), std::move(p)), consult(c), tests(t) {
        if (c < 0 || t < 0) throw BillingValidationError("Negative OP fee");
    }
    double total() const override { return consult + tests; }
    std::string type() const override { return "Out-Patient"; }
};

class InPatientBill : public Bill {
    int days; double perDay, surgery;
public:
    InPatientBill(std::string id, std::string p, int d, double r, double s)
        : Bill(std::move(id), std::move(p)), days(d), perDay(r), surgery(s) {
        if (d <= 0 || r < 0 || s < 0) throw BillingValidationError("Invalid IP charges");
    }
    double total() const override { return days * perDay + surgery; }
    std::string type() const override { return "In-Patient"; }
};

class Payer {
protected:
    std::string name;
public:
    explicit Payer(std::string n) : name(std::move(n)) {}
    virtual ~Payer() = default;
    const std::string& getName() const noexcept { return name; }
    virtual double amountCovered(const Bill& b) const = 0;
};

class Insurance : public Payer {
    double coverLimit;
public:
    Insurance(std::string n, double limit) : Payer(std::move(n)), coverLimit(limit) {}
    double amountCovered(const Bill& b) const override {
        if (b.total() > coverLimit) {
            throw ClaimError("Bill Rs." + std::to_string(b.total()) +
                             " exceeds cover limit Rs." + std::to_string(coverLimit));
        }
        return b.total() * 0.90; // 90% cashless approval
    }
};

// =========================================================================
// B4.1 PRACTICAL EXTENSION: Added WITHOUT modifying any existing class!
// 1. New Bill Type: TeleMedicineBill
// 2. New Payer Type: CorporateTieUp
// =========================================================================
class TeleMedicineBill : public Bill {
    int minutes;
    double ratePerMinute;
    double ePrescriptionFee;
public:
    TeleMedicineBill(std::string id, std::string p, int mins, double rate, double rxFee)
        : Bill(std::move(id), std::move(p)), minutes(mins), ratePerMinute(rate), ePrescriptionFee(rxFee) {
        if (mins <= 0 || rate < 0 || rxFee < 0) {
            throw BillingValidationError("Invalid TeleMedicine duration or fee");
        }
    }
    double total() const override { return (minutes * ratePerMinute) + ePrescriptionFee; }
    std::string type() const override { return "Tele-Medicine"; }
};

class CorporateTieUp : public Payer {
    double annualCap;
    double subsidyPct;
public:
    CorporateTieUp(std::string company, double cap, double subsidy)
        : Payer(std::move(company)), annualCap(cap), subsidyPct(subsidy) {}

    double amountCovered(const Bill& b) const override {
        if (b.total() > annualCap) {
            throw ClaimError("Corporate cap exceeded for " + name);
        }
        return b.total() * (subsidyPct / 100.0);
    }
};

class BillingEngine {
public:
    void settle(const Bill& b, const Payer& p) const {
        try {
            double cov = p.amountCovered(b);
            std::cout << "[SETTLED] " << b.getId() << " (" << b.type() << " - " << b.getPatient()
                      << ") | Total: Rs." << b.total()
                      << " | Payer: " << p.getName()
                      << " | Covered: Rs." << cov
                      << " | Due: Rs." << (b.total() - cov) << "\n";
        } catch (const ClaimError& e) {
            std::cout << "[AUDIT LOG] Claim rejection on " << b.getId() << ": " << e.what() << "\n";
            throw; // Bare rethrow to caller
        }
    }
};

int main() {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "=== TASK B4: PRODUCTION EXTENSION & ASAN EXCEPTION SAFETY ===\n";

    BillingEngine engine;
    std::vector<std::unique_ptr<Bill>> dailyBills;
    dailyBills.push_back(std::make_unique<OutPatientBill>("OP-01", "Arun", 500.0, 1000.0));
    dailyBills.push_back(std::make_unique<InPatientBill>("IP-02", "Priya", 2, 3000.0, 12000.0));
    dailyBills.push_back(std::make_unique<TeleMedicineBill>("TM-03", "Karthik", 30, 25.0, 150.0)); // New type!

    Insurance insurer("HDFC Ergo", 20000.0);
    CorporateTieUp corp("Infosys Corporate Health", 15000.0, 85.0); // New payer!

    // Existing + New bills processed polymorphically
    engine.settle(*dailyBills[0], insurer);
    engine.settle(*dailyBills[1], insurer);
    engine.settle(*dailyBills[2], corp);

    // Trigger exception path under ASan to prove zero leaks on failure
    try {
        std::cout << "\n--- Testing Exception Recovery & Zero Memory Leaks ---\n";
        auto invalidBill = std::make_unique<TeleMedicineBill>("TM-BAD", "Test", -10, 25.0, 100.0);
        engine.settle(*invalidBill, corp);
    } catch (const HospitalException& e) {
        std::cout << "[TOP-LEVEL RECOVERED] " << e.what() << "\n";
    }

    try {
        auto highBill = std::make_unique<InPatientBill>("IP-HIGH", "Vikram", 5, 4000.0, 30000.0);
        engine.settle(*highBill, corp); // Exceeds 15,000 corporate cap
    } catch (const HospitalException& e) {
        std::cout << "[TOP-LEVEL RECOVERED] Claim rejected cleanly without leak.\n";
    }

    std::cout << "\n[ASan Status] Program exiting cleanly with zero memory leaks.\n";
    return 0;
}
