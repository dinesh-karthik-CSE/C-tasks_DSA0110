#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <variant>
#include <vector>

// 1. Virtual Hierarchy
struct VBill {
    virtual ~VBill() = default;
    virtual double total() const = 0;
};
struct VOutPatient : VBill {
    double consult, tests;
    VOutPatient(double c, double t) : consult(c), tests(t) {}
    double total() const override { return consult + tests; }
};
struct VInPatient : VBill {
    int days; double rate, surgery;
    VInPatient(int d, double r, double s) : days(d), rate(r), surgery(s) {}
    double total() const override { return days * rate + surgery; }
};
struct VDayCare : VBill {
    double pkg;
    explicit VDayCare(double p) : pkg(p) {}
    double total() const override { return pkg; }
};
struct VEmergency : VBill {
    double base, hrs, rate;
    VEmergency(double b, double h, double r) : base(b), hrs(h), rate(r) {}
    double total() const override { return base + hrs * rate; }
};

// 2. Tagged Struct + Switch
enum class BillTag { OP, IP, DC, EM };
struct SwitchBill {
    BillTag tag;
    double a = 0, b = 0, c = 0;
    int days = 0;
    double total() const {
        switch (tag) {
            case BillTag::OP: return a + b;
            case BillTag::IP: return days * a + b;
            case BillTag::DC: return a;
            case BillTag::EM: return a + b * c;
        }
        return 0.0;
    }
};

// 3. std::variant + std::visit (Alternative Concept)
struct VarOP { double consult, tests; };
struct VarIP { int days; double rate, surgery; };
struct VarDC { double pkg; };
struct VarEM { double base, hrs, rate; };
using VariantBill = std::variant<VarOP, VarIP, VarDC, VarEM>;

double variantTotal(const VariantBill& vb) {
    return std::visit([](const auto& item) -> double {
        using T = std::decay_t<decltype(item)>;
        if constexpr (std::is_same_v<T, VarOP>) return item.consult + item.tests;
        else if constexpr (std::is_same_v<T, VarIP>) return item.days * item.rate + item.surgery;
        else if constexpr (std::is_same_v<T, VarDC>) return item.pkg;
        else return item.base + item.hrs * item.rate;
    }, vb);
}

// B3.3 Validation: Return code vs Exceptions
enum class ValCode { OK, NEGATIVE_AMOUNT };

ValCode validateReturnCode(double amt, double& out) {
    if (amt < 0.0) return ValCode::NEGATIVE_AMOUNT;
    out = amt * 1.05;
    return ValCode::OK;
}

double validateException(double amt) {
    if (amt < 0.0) throw std::invalid_argument("negative");
    return amt * 1.05;
}

int main() {
    using Clock = std::chrono::high_resolution_clock;
    std::cout << std::fixed << std::setprecision(3);

    // B3.1: Verify correctness on 10 bills
    std::vector<std::unique_ptr<VBill>> v10;
    std::vector<SwitchBill> s10;
    for (int i = 0; i < 10; ++i) {
        int kind = i % 4;
        if (kind == 0) {
            v10.push_back(std::make_unique<VOutPatient>(500 + i * 10, 1200));
            s10.push_back({BillTag::OP, 500.0 + i * 10, 1200.0, 0.0, 0});
        } else if (kind == 1) {
            v10.push_back(std::make_unique<VInPatient>(3, 2500, 15000 + i * 100));
            s10.push_back({BillTag::IP, 2500.0, 15000.0 + i * 100, 0.0, 3});
        } else if (kind == 2) {
            v10.push_back(std::make_unique<VDayCare>(18000));
            s10.push_back({BillTag::DC, 18000.0, 0.0, 0.0, 0});
        } else {
            v10.push_back(std::make_unique<VEmergency>(3000, 4, 1200));
            s10.push_back({BillTag::EM, 3000.0, 4.0, 1200.0, 0});
        }
    }

    double sumV = 0.0, sumS = 0.0;
    for (const auto& b : v10) sumV += b->total();
    for (const auto& b : s10) sumS += b.total();

    std::cout << "=== B3.1: 10-BILL CORRECTNESS VERIFICATION ===\n";
    std::cout << "Virtual Sum = Rs. " << sumV << " | Switch Sum = Rs. " << sumS
              << " | Match: " << (std::abs(sumV - sumS) < 1e-6 ? "YES" : "NO") << "\n\n";

    std::cout << "=== OBJECT SIZE COMPARISON (sizeof) ===\n";
    std::cout << "sizeof(VOutPatient) = " << sizeof(VOutPatient)
              << " B | sizeof(VInPatient) = " << sizeof(VInPatient)
              << " B | sizeof(SwitchBill) = " << sizeof(SwitchBill)
              << " B | sizeof(VariantBill) = " << sizeof(VariantBill) << " B\n\n";

    // B3.2 + Alternative: Benchmark 1K, 1M, 10M bills (-O2)
    std::cout << "=== B3.2: DISPATCH BENCHMARK (ms, compiled with -O2) ===\n";
    std::cout << std::left << std::setw(14) << "Bill Count"
              << std::setw(18) << "Virtual (ms)"
              << std::setw(18) << "Switch (ms)"
              << "std::variant (ms)\n";
    std::cout << std::string(68, '-') << "\n";

    for (size_t N : {1000ULL, 1000000ULL, 10000000ULL}) {
        std::vector<std::unique_ptr<VBill>> vVec;
        std::vector<SwitchBill> sVec;
        std::vector<VariantBill> varVec;
        vVec.reserve(N); sVec.reserve(N); varVec.reserve(N);

        for (size_t i = 0; i < N; ++i) {
            size_t k = i & 3;
            if (k == 0) {
                vVec.push_back(std::make_unique<VOutPatient>(500, 1200));
                sVec.push_back({BillTag::OP, 500, 1200, 0, 0});
                varVec.emplace_back(VarOP{500, 1200});
            } else if (k == 1) {
                vVec.push_back(std::make_unique<VInPatient>(3, 2500, 15000));
                sVec.push_back({BillTag::IP, 2500, 15000, 0, 3});
                varVec.emplace_back(VarIP{3, 2500, 15000});
            } else if (k == 2) {
                vVec.push_back(std::make_unique<VDayCare>(18000));
                sVec.push_back({BillTag::DC, 18000, 0, 0, 0});
                varVec.emplace_back(VarDC{18000});
            } else {
                vVec.push_back(std::make_unique<VEmergency>(3000, 4, 1200));
                sVec.push_back({BillTag::EM, 3000, 4, 1200, 0});
                varVec.emplace_back(VarEM{3000, 4, 1200});
            }
        }

        volatile double sink = 0.0;
        auto t0 = Clock::now();
        double acc1 = 0.0;
        for (const auto& b : vVec) acc1 += b->total();
        auto t1 = Clock::now();
        sink = acc1;

        double acc2 = 0.0;
        for (const auto& b : sVec) acc2 += b.total();
        auto t2 = Clock::now();
        sink = acc2;

        double acc3 = 0.0;
        for (const auto& b : varVec) acc3 += variantTotal(b);
        auto t3 = Clock::now();
        sink = acc3;
        (void)sink;

        double msV   = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double msS   = std::chrono::duration<double, std::milli>(t2 - t1).count();
        double msVar = std::chrono::duration<double, std::milli>(t3 - t2).count();

        std::cout << std::left << std::setw(14) << N
                  << std::setw(18) << msV
                  << std::setw(18) << msS
                  << msVar << "\n";
    }

    // B3.3: Cost of Exceptions vs Return Codes across 1,000,000 bills
    std::cout << "\n=== B3.3: VALIDATION COST ON 1,000,000 BILLS (ms) ===\n";
    std::cout << std::left << std::setw(18) << "Invalid Rate"
              << std::setw(22) << "Return Code (ms)"
              << "Exceptions (ms)\n";
    std::cout << std::string(56, '-') << "\n";

    const size_t M = 1000000;
    for (int pct : {0, 1, 50}) {
        std::vector<double> inputs(M, 1000.0);
        for (size_t i = 0; i < M; ++i) {
            if (pct == 1 && (i % 100 == 0)) inputs[i] = -10.0;
            else if (pct == 50 && (i % 2 == 0)) inputs[i] = -10.0;
        }

        volatile double sinkVal = 0.0;
        size_t errRC = 0;
        auto r0 = Clock::now();
        for (size_t i = 0; i < M; ++i) {
            double res = 0.0;
            if (validateReturnCode(inputs[i], res) == ValCode::OK) sinkVal += res;
            else ++errRC;
        }
        auto r1 = Clock::now();

        size_t errEx = 0;
        auto e0 = Clock::now();
        for (size_t i = 0; i < M; ++i) {
            try {
                sinkVal += validateException(inputs[i]);
            } catch (const std::invalid_argument&) {
                ++errEx;
            }
        }
        auto e1 = Clock::now();
        (void)sinkVal; (void)errRC; (void)errEx;

        double msRC = std::chrono::duration<double, std::milli>(r1 - r0).count();
        double msEx = std::chrono::duration<double, std::milli>(e1 - e0).count();
        std::cout << std::left << std::setw(18) << (std::to_string(pct) + "% invalid")
                  << std::setw(22) << msRC
                  << msEx << "\n";
    }
    return 0;
}
