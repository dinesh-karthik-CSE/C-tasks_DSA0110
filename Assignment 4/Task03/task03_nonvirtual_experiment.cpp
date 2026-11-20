#include <iostream>
#include <string>

class Address {
    std::string city;
    int pin;
public:
    Address(std::string c, int p) : city(std::move(c)), pin(p) {
        std::cout << "[Ctor] Address (" << city << "-" << pin << ")\n";
    }
    ~Address() { std::cout << "[Dtor] ~Address (" << city << ")\n"; }
};

class Course {
    std::string code;
    int credits;
public:
    Course(std::string c, int cr) : code(std::move(c)), credits(cr) {
        std::cout << "[Ctor] Course (" << code << ")\n";
    }
    ~Course() { std::cout << "[Dtor] ~Course (" << code << ")\n"; }
};

class Person {
protected:
    std::string name;
    int id;
    Address addr;
public:
    Person(std::string n, int i, std::string city, int pin)
        : name(std::move(n)), id(i), addr(std::move(city), pin) {
        std::cout << "[Ctor] Person (" << name << ")\n";
    }
    virtual ~Person() { std::cout << "[Dtor] ~Person (" << name << ")\n"; }
};

class Student : public Person { // Non-virtual
protected:
    double cgpa;
    Course enrolledCourse;
public:
    Student(std::string n, int i, std::string city, int pin, double c, std::string cCode, int cr)
        : Person(std::move(n), i, std::move(city), pin), cgpa(c), enrolledCourse(std::move(cCode), cr) {
        std::cout << "[Ctor] Student\n";
    }
    ~Student() override { std::cout << "[Dtor] ~Student\n"; }
};

class Staff : public Person { // Non-virtual
protected:
    std::string dept;
public:
    Staff(std::string n, int i, std::string city, int pin, std::string d)
        : Person(std::move(n), i, std::move(city), pin), dept(std::move(d)) {
        std::cout << "[Ctor] Staff\n";
    }
    ~Staff() override { std::cout << "[Dtor] ~Staff\n"; }
};

class TeachingAssistant : public Student, public Staff {
    std::string labSection;
public:
    TeachingAssistant(std::string n, int i, std::string city, int pin,
                      double c, std::string cCode, int cr,
                      std::string d, std::string lab)
        : Student(n, i, city, pin, c, std::move(cCode), cr),
          Staff(n, i, city, pin, std::move(d)),
          labSection(std::move(lab)) {
        std::cout << "[Ctor] TeachingAssistant\n";
    }
    ~TeachingAssistant() override { std::cout << "[Dtor] ~TeachingAssistant\n"; }
};

int main() {
    std::cout << "--- Experiment: Without virtual keyword ---\n";
    TeachingAssistant ta("Priya", 301, "Coimbatore", 641001, 9.3, "CS302", 3, "CSE", "B2");
    std::cout << "sizeof(TeachingAssistant) [non-virtual] = " << sizeof(TeachingAssistant) << " bytes\n";
    return 0;
}
