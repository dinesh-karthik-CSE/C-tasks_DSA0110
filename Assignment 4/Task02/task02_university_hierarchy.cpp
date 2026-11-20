#include <iostream>
#include <string>

class Person {
protected:
    std::string name;
    int id;
    std::string phone;

public:
    Person(std::string n, int i, std::string p)
        : name(std::move(n)), id(i), phone(std::move(p)) {}

    virtual ~Person() = default;

    const std::string& getName() const { return name; }
    int getId() const { return id; }
    const std::string& getPhone() const { return phone; }

    virtual void display() const {
        std::cout << "ID: " << id << " | Name: " << name << " | Phone: " << phone;
    }
};

class Student : virtual public Person {
protected:
    std::string programme;
    double cgpa;

public:
    Student(std::string n, int i, std::string p, std::string prog, double c)
        : Person(std::move(n), i, std::move(p)), programme(std::move(prog)), cgpa(c) {}

    void display() const override {
        Person::display();
        std::cout << " | Programme: " << programme << " | CGPA: " << cgpa;
    }
};

class Staff : virtual public Person {
protected:
    std::string department;
    double salary;

public:
    Staff(std::string n, int i, std::string p, std::string dept, double sal)
        : Person(std::move(n), i, std::move(p)), department(std::move(dept)), salary(sal) {}

    void display() const override {
        Person::display();
        std::cout << " | Dept: " << department << " | Salary: Rs. " << salary;
    }
};

class TeachingAssistant : public Student, public Staff {
private:
    std::string assignedCourse;

public:
    TeachingAssistant(std::string n, int i, std::string p,
                      std::string prog, double c,
                      std::string dept, double sal,
                      std::string course)
        : Person(std::move(n), i, std::move(p)),
          Student(name, id, phone, std::move(prog), c),
          Staff(name, id, phone, std::move(dept), sal),
          assignedCourse(std::move(course)) {}

    void display() const override {
        Person::display(); // Unambiguous single Person sub-object!
        std::cout << " | Programme: " << programme << " | CGPA: " << cgpa
                  << " | Dept: " << department << " | Stipend: Rs. " << salary
                  << " | TA Course: " << assignedCourse;
    }
};

class Fellowship {
private:
    std::string agency;
    double grantAmount;

public:
    Fellowship(std::string ag, double amt) : agency(std::move(ag)), grantAmount(amt) {}
    void display() const {
        std::cout << "Fellowship: " << agency << " (Rs. " << grantAmount << "/mo)";
    }
};

class ResearchScholar : public Student {
private:
    Fellowship fellowship; // Composition (Has-a)

public:
    ResearchScholar(std::string n, int i, std::string p,
                    std::string prog, double c,
                    std::string agency, double grant)
        : Person(std::move(n), i, std::move(p)),
          Student(name, id, phone, std::move(prog), c),
          fellowship(std::move(agency), grant) {}

    void display() const override {
        Student::display();
        std::cout << " | ";
        fellowship.display();
    }
};

int main() {
    Student s("Arjun K", 1001, "9876543210", "B.E. CSE", 8.95);
    Staff st("Prof. Ramesh", 2001, "9876500001", "CSE", 95000.0);
    TeachingAssistant ta("Divya S", 3001, "9876511111", "M.E. CSE", 9.20, "CSE", 25000.0, "DSA01");
    ResearchScholar rs("Karthik V", 4001, "9876522222", "Ph.D. AI", 9.40, "CSIR-JRF", 37000.0);

    std::cout << "=== UNIVERSITY PEOPLE DIRECTORY ===\n";
    s.display();  std::cout << "\n";
    st.display(); std::cout << "\n";
    ta.display(); std::cout << "\n";
    rs.display(); std::cout << "\n\n";

    std::cout << "sizeof(Person)            = " << sizeof(Person) << " bytes\n";
    std::cout << "sizeof(Student)           = " << sizeof(Student) << " bytes\n";
    std::cout << "sizeof(Staff)             = " << sizeof(Staff) << " bytes\n";
    std::cout << "sizeof(TeachingAssistant) = " << sizeof(TeachingAssistant) << " bytes\n";
    return 0;
}
