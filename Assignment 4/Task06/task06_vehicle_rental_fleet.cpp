#include <iomanip>
#include <iostream>
#include <string>

// Member Class (Composition: Has-a) for Service Date
class Date {
    int day, month, year;
public:
    Date(int d = 1, int m = 1, int y = 2026) : day(d), month(m), year(y) {}
    std::string str() const {
        return (day < 10 ? "0" : "") + std::to_string(day) + "/" +
               (month < 10 ? "0" : "") + std::to_string(month) + "/" +
               std::to_string(year);
    }
};

// Abstract Base Class
class Vehicle {
protected:
    std::string regNo;
    double dailyRate;
    bool rented;
    int bookedDays;

public:
    Vehicle(std::string reg, double rate)
        : regNo(std::move(reg)), dailyRate(rate), rented(false), bookedDays(0) {}

    virtual ~Vehicle() = default;

    const std::string& getRegNo() const { return regNo; }
    bool isRented() const { return rented; }

    bool rentOut(int days) {
        if (rented || days <= 0) return false;
        rented = true;
        bookedDays = days;
        return true;
    }

    // Returns total payable amount including late penalty (50% surcharge per extra day)
    double returnVehicle(int actualDays, double energyUsed = 0.0) {
        if (!rented) return -1.0;
        int extraDays = (actualDays > bookedDays) ? (actualDays - bookedDays) : 0;
        double baseCharge = computeCharge(actualDays, energyUsed);
        double lateFee = extraDays * (dailyRate * 0.50);
        rented = false;
        bookedDays = 0;
        return baseCharge + lateFee;
    }

    virtual std::string category() const = 0;
    virtual double computeCharge(int days, double energyUsed) const = 0;

    virtual void printDetails() const {
        std::cout << std::left << std::setw(12) << regNo
                  << std::setw(16) << category()
                  << "Rs. " << std::setw(9) << std::fixed << std::setprecision(2) << dailyRate
                  << std::setw(12) << (rented ? "RENTED" : "AVAILABLE");
    }
};

// Concrete Class 1: Non-motorized Bicycle
class Bicycle : public Vehicle {
    bool hasGear;
public:
    Bicycle(std::string reg, double rate, bool gear)
        : Vehicle(std::move(reg), rate), hasGear(gear) {}

    std::string category() const override { return "Bicycle"; }

    // Flat daily rate + Rs. 20/day accessory fee if geared
    double computeCharge(int days, double /*energyUsed*/) const override {
        return days * (dailyRate + (hasGear ? 20.0 : 0.0));
    }

    void printDetails() const override {
        Vehicle::printDetails();
        std::cout << "Gear: " << (hasGear ? "Yes" : "No") << "\n";
    }
};

// Intermediate Base Class for Motorized Vehicles
class MotorVehicle : public Vehicle {
protected:
    double energyLevelPercent; // Fuel % or Charge %
    Date serviceDueDate;       // Composition (Has-a)

public:
    MotorVehicle(std::string reg, double rate, double level, Date due)
        : Vehicle(std::move(reg), rate), energyLevelPercent(level), serviceDueDate(due) {}

    void printDetails() const override {
        Vehicle::printDetails();
        std::cout << "Level: " << std::setw(5) << energyLevelPercent << "% | Service: " << serviceDueDate.str();
    }
};

// Concrete Class 2: Two-Wheeler (Petrol)
class TwoWheeler : public MotorVehicle {
    static constexpr double PETROL_RATE_PER_LITRE = 102.0;
public:
    TwoWheeler(std::string reg, double rate, double fuelPct, Date due)
        : MotorVehicle(std::move(reg), rate, fuelPct, due) {}

    std::string category() const override { return "Two-Wheeler"; }

    double computeCharge(int days, double litresUsed) const override {
        return (days * dailyRate) + (litresUsed * PETROL_RATE_PER_LITRE);
    }

    void printDetails() const override {
        MotorVehicle::printDetails();
        std::cout << " | Fuel: Petrol\n";
    }
};

// Concrete Class 3: Car (Petrol/Diesel)
class Car : public MotorVehicle {
protected:
    int seatingCapacity;
    static constexpr double FUEL_RATE_PER_LITRE = 102.0;
public:
    Car(std::string reg, double rate, double fuelPct, Date due, int seats)
        : MotorVehicle(std::move(reg), rate, fuelPct, due), seatingCapacity(seats) {}

    std::string category() const override { return "Car"; }

    // Includes Rs. 150/day insurance fee for 5+ seaters
    double computeCharge(int days, double litresUsed) const override {
        double seatInsurance = (seatingCapacity > 5) ? (150.0 * days) : 0.0;
        return (days * dailyRate) + seatInsurance + (litresUsed * FUEL_RATE_PER_LITRE);
    }

    void printDetails() const override {
        MotorVehicle::printDetails();
        std::cout << " | Seats: " << seatingCapacity << " | Fuel: Petrol\n";
    }
};

// Concrete Class 4: Electric Car (charged per kWh instead of per litre)
class ElectricCar : public MotorVehicle {
private:
    int seatingCapacity;
    double batteryCapacityKWh;
    static constexpr double RATE_PER_KWH = 12.5;

public:
    ElectricCar(std::string reg, double rate, double chargePct, Date due, int seats, double batteryKWh)
        : MotorVehicle(std::move(reg), rate, chargePct, due),
          seatingCapacity(seats), batteryCapacityKWh(batteryKWh) {}

    std::string category() const override { return "Electric Car"; }

    double computeCharge(int days, double kwhUsed) const override {
        return (days * dailyRate) + (kwhUsed * RATE_PER_KWH);
    }

    void printDetails() const override {
        MotorVehicle::printDetails();
        std::cout << " | Seats: " << seatingCapacity << " | Battery: " << batteryCapacityKWh << " kWh\n";
    }
};

// Stage 5 Extension: New Vehicle Type added WITHOUT modifying any existing class!
class ElectricBus : public MotorVehicle {
private:
    int passengerCapacity;
    double batteryCapacityKWh;
    static constexpr double COMMERCIAL_KWH_RATE = 10.0;

public:
    ElectricBus(std::string reg, double rate, double chargePct, Date due, int passengers, double batteryKWh)
        : MotorVehicle(std::move(reg), rate, chargePct, due),
          passengerCapacity(passengers), batteryCapacityKWh(batteryKWh) {}

    std::string category() const override { return "Electric Bus"; }

    double computeCharge(int days, double kwhUsed) const override {
        return (days * dailyRate) + (kwhUsed * COMMERCIAL_KWH_RATE) + 500.0; // Commercial permit fee
    }

    void printDetails() const override {
        MotorVehicle::printDetails();
        std::cout << " | Pax: " << passengerCapacity << " | Battery: " << batteryCapacityKWh << " kWh\n";
    }
};

// Fleet Manager storing array of base-class pointers
class RentalFleet {
    static constexpr int MAX_VEHICLES = 10;
    Vehicle* fleet[MAX_VEHICLES];
    int count = 0;

public:
    ~RentalFleet() {
        for (int i = 0; i < count; ++i) {
            delete fleet[i];
        }
    }

    void addVehicle(Vehicle* v) {
        if (count < MAX_VEHICLES) {
            fleet[count++] = v;
        }
    }

    Vehicle* findByReg(const std::string& reg) const {
        for (int i = 0; i < count; ++i) {
            if (fleet[i]->getRegNo() == reg) return fleet[i];
        }
        return nullptr;
    }

    void listAll() const {
        std::cout << "---------------------------------------------------------------------------------------\n";
        for (int i = 0; i < count; ++i) {
            fleet[i]->printDetails();
        }
        std::cout << "---------------------------------------------------------------------------------------\n";
    }
};

int main() {
    RentalFleet agency;
    agency.addVehicle(new Bicycle("TN01-BK01", 150.0, true));
    agency.addVehicle(new TwoWheeler("TN01-TW22", 450.0, 85.0, Date(15, 12, 2026)));
    agency.addVehicle(new Car("TN01-CR09", 1800.0, 70.0, Date(20, 1, 2027), 7));
    agency.addVehicle(new ElectricCar("TN01-EV77", 2400.0, 92.0, Date(10, 2, 2027), 5, 60.5));
    // Added new vehicle type dynamically without touching existing classes
    agency.addVehicle(new ElectricBus("TN01-EB99", 6500.0, 95.0, Date(30, 3, 2027), 40, 250.0));

    std::cout << "=== INITIAL FLEET INVENTORY ===\n";
    agency.listAll();

    // Rent out all 5 vehicle types
    std::cout << "\n=== RENTING VEHICLES ===\n";
    const char* regs[] = {"TN01-BK01", "TN01-TW22", "TN01-CR09", "TN01-EV77", "TN01-EB99"};
    int bookedDays[]   = {2, 3, 4, 2, 3};
    for (int i = 0; i < 5; ++i) {
        Vehicle* v = agency.findByReg(regs[i]);
        if (v && v->rentOut(bookedDays[i])) {
            std::cout << "Rented " << v->getRegNo() << " (" << v->category() << ") for " << bookedDays[i] << " days.\n";
        }
    }

    std::cout << "\n=== RETURNING VEHICLES (INCLUDING LATE RETURN DEMO) ===\n";
    std::cout << std::fixed << std::setprecision(2);
    // Electric Car booked for 2 days, returned after 4 days (2 days late!) + 48.0 kWh used
    Vehicle* ev = agency.findByReg("TN01-EV77");
    double evBill = ev->returnVehicle(4, 48.0);
    std::cout << "Returned TN01-EV77 (Booked 2d, Actual 4d [2d Late], 48 kWh): Total Bill = Rs. " << evBill << "\n";

    // Electric Bus returned on time (3 days, 180 kWh)
    Vehicle* eb = agency.findByReg("TN01-EB99");
    double ebBill = eb->returnVehicle(3, 180.0);
    std::cout << "Returned TN01-EB99 (Booked 3d, Actual 3d [On Time], 180 kWh): Total Bill = Rs. " << ebBill << "\n";

    // Bicycle returned on time (2 days)
    Vehicle* bk = agency.findByReg("TN01-BK01");
    double bkBill = bk->returnVehicle(2, 0.0);
    std::cout << "Returned TN01-BK01 (Booked 2d, Actual 2d [On Time]): Total Bill = Rs. " << bkBill << "\n";

    std::cout << "\n=== UPDATED FLEET STATUS ===\n";
    agency.listAll();
    return 0;
}
