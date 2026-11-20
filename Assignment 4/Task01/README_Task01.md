# Task 01 – Class Design Review (Diagnose and Repair Duplicated Classes)

## Task 1.1 – Diagnose the Design (6 Design Deficiencies Identified)

1. **Duplicated Data Members & Logic (Violation of DRY):** `name`, `id`, `basic`, and `da` are copy-pasted across `Professor` and `LabAssistant`, and the Dearness Allowance formula (`basic + basic * 0.42`) is duplicated verbatim. Any policy change in DA calculation requires hunting through multiple classes.
2. **Broken Encapsulation (All `public` Fields):** Every attribute (`basic`, `hours`, `rate`, `id`) is declared `public`. External code can arbitrarily mutate salaries, set negative hours, or corrupt employee IDs without validation.
3. **Inconsistent Naming Conventions for Identical Concepts:** `Professor` and `LabAssistant` identify employees using `name` (string) and `id` (int), whereas `GuestFaculty` uses `nm` (string) and `code` (int) for the exact same domain attributes, breaking interface uniformity.
4. **Dead / Unused Member Variables (`da`):** Both `Professor` and `LabAssistant` declare `double da;`, yet neither `pay()` method reads or initializes `da`—instead hardcoding `basic * 0.42`. This wastes 8 bytes per object and creates state inconsistency.
5. **Hardcoded Magic Number (`0.42`):** The DA percentage `0.42` is embedded directly inside arithmetic expressions across multiple classes instead of being defined as a named `static constexpr double DA_RATE = 0.42;` constant in a shared base class.
6. **Inability to Process Polymorphically & High Extension Cost:** Because the three classes share no common base class or virtual interface, the payroll system cannot store all staff in a single `std::vector<Staff*>` or compute total payroll in one loop. Adding a new staff category (e.g., `VisitingFellow`) forces duplicate boilerplate and separate processing loops (violating the Open/Closed Principle).

---

## Task 1.2 – Repair the Design & UML Class Diagram

### UML Class Diagram (ASCII)
```text
                  +--------------------------------------+
                  |          <<abstract>>                |
                  |               Staff                  |
                  +--------------------------------------+
                  | # static constexpr DA_RATE = 0.42    |
                  | # name : string                      |
                  | # id : int                           |
                  +--------------------------------------+
                  | + Staff(name, id)                    |
                  | + virtual ~Staff()                   |
                  | + virtual role() : string = 0        |
                  | + virtual pay() : double = 0         |
                  +--------------------------------------+
                             ^                ^
                             | (public)       | (public)
              +--------------+                +------------------+
              |                                                  |
+---------------------------+                      +---------------------------+
|       SalariedStaff       |                      |       GuestFaculty        |
+---------------------------+                      +---------------------------+
| # basic : double          |                      | - hours : double          |
+---------------------------+                      | - hourlyRate : double     |
| + SalariedStaff(...)      |                      +---------------------------+
| # computeDA() : double    |                      | + GuestFaculty(...)       |
+---------------------------+                      | + role() : string         |
       ^             ^                             | + pay() : double          |
       |             |                             +---------------------------+
+--------------+   +---------------+
|  Professor   |   | LabAssistant  |
+--------------+   +---------------+
| - research   |   |               |
+--------------+   +---------------+
| + role()     |   | + role()      |
| + pay()      |   | + pay()       |
+--------------+   +---------------+
```

---

## Task 1.3 – Design Defence (82 Words)

Grouping shared identity fields (`name`, `id`) and the `DA_RATE` constant into an abstract `Staff` base class—with `SalariedStaff` factoring out `basic` and `computeDA()`—eliminates code duplication and enforces encapsulation. Adding a new staff category now requires **only creating a new derived class** that overrides `role()` and `pay()`. Existing working classes (`Staff`, `Professor`, `LabAssistant`, `GuestFaculty`) and the polymorphic payroll loop remain completely untouched, strictly satisfying the **Open/Closed Principle** and eliminating regression risks.

---

## Alternative Concept and Complexity Analysis

### Alternative Approach: Single `Staff` Class with `enum StaffType` and `switch` inside `pay()`
```cpp
enum class StaffType { PROFESSOR, LAB_ASSISTANT, GUEST_FACULTY };
class MonolithicStaff {
    StaffType type;
    std::string name; int id;
    double basic = 0, research = 0, hours = 0, rate = 0;
public:
    double pay() const {
        switch (type) {
            case StaffType::PROFESSOR:     return basic + basic * 0.42 + research;
            case StaffType::LAB_ASSISTANT: return basic + basic * 0.42;
            case StaffType::GUEST_FACULTY: return hours * rate;
        }
        return 0.0;
    }
};
```

### Complexity & Maintainability Comparison

| Metric | Inheritance Hierarchy (`virtual pay()`) | Single Class (`enum` + `switch`) |
| :--- | :--- | :--- |
| **Time Complexity per `pay()`** | $\mathcal{O}(1)$ — single indirect vtable pointer lookup (~1–2 ns overhead) | $\mathcal{O}(1)$ — jump table / branch comparison |
| **Space Complexity per Object** | Tightly tailored: `LabAssistant` stores only `name`, `id`, `basic`, and 1 `vptr` (8 B); `GuestFaculty` stores no `basic`/`research`. | Bloated: Every object carries **all fields** (`basic`, `research`, `hours`, `rate` + `enum`), wasting 16–24 unused bytes per instance. |
| **Places Changed to Add a New Staff Type** | **1 place** (new subclass only; zero edits to existing code) | **4+ places** (`enum`, constructors, unused fields in class, and `switch` statement inside `pay()` and `role()`). |
