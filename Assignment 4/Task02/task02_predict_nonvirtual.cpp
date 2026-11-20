#include <iostream>
struct Person { std::string name = "Test"; Person() { std::cout << "Person "; } };
struct Student : Person { Student() { std::cout << "Student "; } };
struct Staff : Person { Staff() { std::cout << "Staff "; } };
struct TeachingAssistant : Student, Staff {
    TeachingAssistant() { std::cout << "TA "; }
};
int main()
{
    TeachingAssistant ta;
    std::cout << '\n';
    // std::cout << ta.name; // Uncommenting triggers: error: request for member 'name' is ambiguous
}
