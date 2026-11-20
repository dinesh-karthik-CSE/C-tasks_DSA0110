#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

class Staff {
protected:
    static constexpr double DA_RATE = 0.42;
    std::string name;
    int id;

public:
    Staff(std::string staffName, int staffId)
        : name(std::move(staffName)), id(staffId) {}

    virtual ~Staff() = default;

    int getId() const { return id; }
    const std::string& getName() const { return name; }

    virtual std::string role() const = 0;
    virtual double pay() const = 0;
};

class SalariedStaff : public Staff {
protected:
    double basic;

public:
    SalariedStaff(std::string staffName, int staffId, double basicPay)
        : Staff(std::move(staffName), staffId), basic(basicPay) {}

    double computeDA() const { return basic * DA_RATE; }
};

class Professor : public SalariedStaff {
private:
    double researchAllowance;

public:
    Professor(std::string staffName, int staffId, double basicPay, double research)
        : SalariedStaff(std::move(staffName), staffId, basicPay),
          researchAllowance(research) {}

    std::string role() const override { return "Professor"; }
    double pay() const override { return basic + computeDA() + researchAllowance; }
};

class LabAssistant : public SalariedStaff {
public:
    LabAssistant(std::string staffName, int staffId, double basicPay)
        : SalariedStaff(std::move(staffName), staffId, basicPay) {}

    std::string role() const override { return "Lab Assistant"; }
    double pay() const override { return basic + computeDA(); }
};

class GuestFaculty : public Staff {
private:
    double hours;
    double hourlyRate;

public:
    GuestFaculty(std::string staffName, int staffId, double hrs, double rate)
        : Staff(std::move(staffName), staffId), hours(hrs), hourlyRate(rate) {}

    std::string role() const override { return "Guest Faculty"; }
    double pay() const override { return hours * hourlyRate; }
};

// Demonstration of Open/Closed Principle: adding a new staff category without modifying existing classes
class VisitingFellow : public Staff {
private:
    double stipend;

public:
    VisitingFellow(std::string staffName, int staffId, double monthlyStipend)
        : Staff(std::move(staffName), staffId), stipend(monthlyStipend) {}

    std::string role() const override { return "Visiting Fellow"; }
    double pay() const override { return stipend; }
};

int main() {
    std::vector<Staff*> payroll;
    payroll.push_back(new Professor("Dr. Ananya Rao", 101, 80000.0, 15000.0));
    payroll.push_back(new LabAssistant("Karthik M", 205, 35000.0));
    payroll.push_back(new GuestFaculty("Meera Nair", 312, 40.0, 1200.0));
    payroll.push_back(new VisitingFellow("Dr. Vikram Sen", 408, 55000.0));

    std::cout << "=== UNIVERSITY UNIFIED PAYROLL REPORT ===\n";
    std::cout << std::left << std::setw(6) << "ID"
              << std::setw(20) << "Name"
              << std::setw(18) << "Role"
              << std::right << std::setw(12) << "Net Pay\n";
    std::cout << std::string(56, '-') << "\n";

    double totalOutflow = 0.0;
    for (const Staff* member : payroll) {
        double p = member->pay();
        totalOutflow += p;
        std::cout << std::left << std::setw(6) << member->getId()
                  << std::setw(20) << member->getName()
                  << std::setw(18) << member->role()
                  << std::right << std::setw(12) << std::fixed << std::setprecision(2) << p << "\n";
    }

    std::cout << std::string(56, '-') << "\n";
    std::cout << "Total Payroll Outflow: Rs. " << std::fixed << std::setprecision(2) << totalOutflow << "\n";

    for (Staff* member : payroll) {
        delete member;
    }
    return 0;
}
