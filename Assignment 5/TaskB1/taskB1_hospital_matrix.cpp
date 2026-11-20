#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Bill {
protected:
    std::string billId;
    std::string patientName;
    std::string packageCode;

public:
    Bill(std::string id, std::string patient, std::string pkg = "GEN")
        : billId(std::move(id)), patientName(std::move(patient)), packageCode(std::move(pkg)) {}

    virtual ~Bill() = default;

    const std::string& getBillId() const noexcept { return billId; }
    const std::string& getPatientName() const noexcept { return patientName; }
    const std::string& getPackageCode() const noexcept { return packageCode; }

    virtual double total() const = 0;
    virtual std::string type() const = 0;
};

class OutPatientBill : public Bill {
    double consultation, tests;
public:
    OutPatientBill(std::string id, std::string p, double c, double t)
        : Bill(std::move(id), std::move(p), "OP-STD"), consultation(c), tests(t) {}
    double total() const override { return consultation + tests; }
    std::string type() const override { return "Out-Patient"; }
};

class InPatientBill : public Bill {
    int days;
    double roomPerDay, procedures, medicines;
public:
    InPatientBill(std::string id, std::string p, int d, double r, double proc, double med)
        : Bill(std::move(id), std::move(p), "IP-GEN"),
          days(d), roomPerDay(r), procedures(proc), medicines(med) {}
    double total() const override { return (days * roomPerDay) + procedures + medicines; }
    std::string type() const override { return "In-Patient"; }
};

class DayCareBill : public Bill {
    double packageFee;
public:
    DayCareBill(std::string id, std::string p, std::string pkg, double fee)
        : Bill(std::move(id), std::move(p), std::move(pkg)), packageFee(fee) {}
    double total() const override { return packageFee; }
    std::string type() const override { return "Day-Care"; }
};

class EmergencyBill : public Bill {
    double baseCharge, hours, hourlyRate;
public:
    EmergencyBill(std::string id, std::string p, double base, double hrs, double rate)
        : Bill(std::move(id), std::move(p), "EMER-24"),
          baseCharge(base), hours(hrs), hourlyRate(rate) {}
    double total() const override { return baseCharge + (hours * hourlyRate); }
    std::string type() const override { return "Emergency"; }
};

class Payer {
protected:
    std::string payerName;
public:
    explicit Payer(std::string name) : payerName(std::move(name)) {}
    virtual ~Payer() = default;
    const std::string& getName() const noexcept { return payerName; }
    virtual double amountCovered(const Bill& bill) const = 0;
};

class SelfPay : public Payer {
public:
    SelfPay() : Payer("Self-Pay") {}
    double amountCovered(const Bill& /*bill*/) const override { return 0.0; }
};

class Insurance : public Payer {
    double coverLimit;
    double coPayPct;
public:
    Insurance(std::string insurer, double limit, double coPay)
        : Payer(std::move(insurer)), coverLimit(limit), coPayPct(coPay) {}

    double amountCovered(const Bill& bill) const override {
        double eligible = std::min(bill.total(), coverLimit);
        return eligible * (1.0 - coPayPct / 100.0);
    }
};

class GovernmentScheme : public Payer {
    std::unordered_map<std::string, double> packageRates;
public:
    explicit GovernmentScheme(std::string schemeName) : Payer(std::move(schemeName)) {
        packageRates["OP-STD"]   = 1000.0;
        packageRates["IP-GEN"]   = 35000.0;
        packageRates["CATARACT"] = 18000.0;
        packageRates["EMER-24"]  = 12000.0;
    }

    double amountCovered(const Bill& bill) const override {
        auto it = packageRates.find(bill.getPackageCode());
        if (it == packageRates.end()) return 0.0;
        return std::min(bill.total(), it->second);
    }
};

int main() {
    std::vector<std::unique_ptr<Bill>> bills;
    bills.push_back(std::make_unique<OutPatientBill>("B101", "Ravi Kumar", 600.0, 1400.0));
    bills.push_back(std::make_unique<InPatientBill>("B102", "Meena Devi", 4, 3000.0, 25000.0, 5000.0));
    bills.push_back(std::make_unique<DayCareBill>("B103", "Suresh R", "CATARACT", 22000.0));
    bills.push_back(std::make_unique<EmergencyBill>("B104", "Anitha K", 4500.0, 5.0, 1500.0));

    std::vector<std::unique_ptr<Payer>> payers;
    payers.push_back(std::make_unique<SelfPay>());
    payers.push_back(std::make_unique<Insurance>("StarHealth (30k, 10% copay)", 30000.0, 10.0));
    payers.push_back(std::make_unique<GovernmentScheme>("CMCHIS Govt Scheme"));

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "=== BILL x PAYER SETTLEMENT MATRIX ===\n";
    std::cout << std::left << std::setw(8) << "BillID"
              << std::setw(14) << "Category"
              << std::setw(11) << "GrossBill"
              << std::setw(28) << "Payer"
              << std::setw(11) << "Covered"
              << "PatientDue\n";
    std::cout << std::string(82, '-') << "\n";

    for (const auto& b : bills) {
        double gross = b->total();
        for (const auto& p : payers) {
            double covered = p->amountCovered(*b);
            double due = gross - covered;
            std::cout << std::left << std::setw(8) << b->getBillId()
                      << std::setw(14) << b->type()
                      << std::setw(11) << gross
                      << std::setw(28) << p->getName()
                      << std::setw(11) << covered
                      << due << "\n";
        }
        std::cout << std::string(82, '-') << "\n";
    }
    return 0;
}
