# ASSIGNMENT 4 – CO4: DESIGNING REUSABLE CLASS HIERARCHIES
**Course:** Object Oriented Programming with C++ (`DSA01`) | **Bloom's Level:** K5 – Evaluate
**Student Name:** `<Your Name>` | **Register Number:** `<Your Register Number>`
**GitHub Repository:** `https://github.com/<username>/DSA01-<RegisterNumber>-Assignment4`

---

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


---

# Task 02 – From Requirement to Hierarchy (University People Management)

## Task 2.1 – Relationship Classification & Inheritance Types

| Entity / Relationship | Classification | Type of Inheritance / Composition | Justification |
| :--- | :--- | :--- | :--- |
| `Student` $
ightarrow$ `Person` | **Is-a** | Hierarchical / Virtual Single Inheritance | Every student is a person with common attributes (`name`, `id`, `phone`). |
| `Staff` $
ightarrow$ `Person` | **Is-a** | Hierarchical / Virtual Single Inheritance | Every staff member is also a person sharing identity attributes. |
| `TeachingAssistant` $
ightarrow$ `Student`, `Staff` | **Is-a** (both) | **Multiple + Hybrid (Diamond)** Inheritance | A TA is simultaneously a student and a staff member. |
| `ResearchScholar` $
ightarrow$ `Student` | **Is-a** | **Multilevel** Inheritance (`Person` $
ightarrow$ `Student` $
ightarrow$ `ResearchScholar`) | A research scholar is a specialized student. |
| `ResearchScholar` $
ightarrow$ `Fellowship` | **Has-a** | **Composition (Member Class)** | A fellowship (grant agency, stipend amount) is an external award held by the scholar, not a person subtype. |

---

## Task 2.2 – Resolving the Diamond Problem

Without `virtual` inheritance, `TeachingAssistant` inherits from both `Student` and `Staff`, each of which brings its own independent `Person` sub-object. This causes two critical defects:
1. **Data Duplication:** Two copies of `name`, `id`, and `phone` exist inside every `TeachingAssistant` object, wasting memory and allowing the two copies to fall out of sync.
2. **Member Access Ambiguity:** Accessing `ta.getName()` or `ta.name` fails at compile time (`error: request for member 'name' is ambiguous`) because the compiler cannot tell whether to resolve `Student::Person::name` or `Staff::Person::name`.

**How Virtual Base Classes Solve It:** Declaring `class Student : virtual public Person` and `class Staff : virtual public Person` instructs the compiler to share a **single, unified `Person` sub-object** across all virtually derived paths in the most-derived class (`TeachingAssistant`). Both `Student` and `Staff` hold hidden virtual base table (`vbptr`) offsets pointing to that single `Person` instance, eliminating duplication and ambiguity.

---

## Task 2.3 – Access Specifier Decisions

* **`Person` data members (`name`, `id`, `phone`):** Declared `protected` so derived classes (`Student`, `Staff`, `TeachingAssistant`) can directly access them while preventing arbitrary external modification.
* **Inheritance mode (`public` inheritance):** `Student`, `Staff`, `TeachingAssistant`, and `ResearchScholar` all inherit **`public`ly** because they satisfy true **Is-a** relationships (Liskov Substitution Principle), allowing any derived object to be passed wherever a `Person*` or `Person&` is expected.

---

## Task 2.4 – Access Visibility Table (`Person` $
ightarrow$ `Student`)

| Member Access in `Person` | `public` Inheritance (`class Student : public Person`) | `protected` Inheritance (`class Student : protected Person`) | `private` Inheritance (`class Student : private Person`) |
| :--- | :--- | :--- | :--- |
| **`public` member** | `public` in `Student` | `protected` in `Student` | `private` in `Student` |
| **`protected` member** | `protected` in `Student` | `protected` in `Student` | `private` in `Student` |
| **`private` member** | **Inaccessible** (hidden in `Student`) | **Inaccessible** (hidden in `Student`) | **Inaccessible** (hidden in `Student`) |

---

## Predict the Output

### 1. With `virtual` Base Classes
* **Predicted Output (Written Before Execution):**
  ```text
  Person Student Staff TA 
  ```
* **Actual Output:**
  ```text
  Person Student Staff TA 
  ```
* **Explanation:** In C++, virtual base classes are always initialized **first** (before non-virtual base classes) by the most-derived class (`TeachingAssistant`), and only **once**. Next, non-virtual direct base classes are constructed left-to-right in declaration order (`Student`, then `Staff`), skipping their own `Person()` calls. Finally, `TeachingAssistant()` executes.

### 2. Without `virtual` Keyword Removed
* **Predicted Output (Written Before Execution):**
  ```text
  Person Student Person Staff TA 
  ```
* **Actual Output & Compiler Diagnostics:**
  * Execution output: `Person Student Person Staff TA `
  * If any member of `Person` (such as `ta.name`) is accessed directly on `ta`, `g++` emits:
    `error: request for member 'name' is ambiguous` (candidates are `Person::name` via `Student` and `Person::name` via `Staff`).
  * Without `virtual`, `Student` constructs its own `Person` sub-object (`Person Student`), and `Staff` constructs a second independent `Person` sub-object (`Person Staff`), followed by `TA`.

---

## Alternative Concept and Complexity Analysis

### Alternative Approach: Composition (`TeachingAssistant` Has-a `Student` and Has-a `Staff`)
```cpp
class TeachingAssistantComposition {
    Student studentProfile; // Contains 1st Person sub-object
    Staff staffProfile;     // Contains 2nd Person sub-object
    std::string assignedCourse;
public:
    const std::string& getName() const { return studentProfile.getName(); } // Manual forwarding
};
```

### Complexity & Layout Comparison

| Dimension | Virtual Inheritance (`Is-a`) | Composition (`Has-a` `Student` + `Staff`) |
| :--- | :--- | :--- |
| **Space per Object (`sizeof`)** | **1 `Person` sub-object** + 2 `vbptr`s (16 B on 64-bit). Measured `sizeof(TeachingAssistant)` = 160 B vs two separate objects (192 B). | **2 full `Person` sub-objects** (`studentProfile.Person` + `staffProfile.Person`), duplicating `name`, `id`, and `phone` (32+ bytes each). |
| **Time to Access `name`** | $\mathcal{O}(1)$ — single indirect `vbptr` offset lookup (`this + vbtable_offset`). | $\mathcal{O}(1)$ — direct member offset (`studentProfile.name`), **no** `vbptr` indirection. |
| **Ambiguity & Polymorphism** | **Zero ambiguity** (`ta.getName()` resolves uniquely); supports `Person* p = &ta;` polymorphism. | **No compiler ambiguity** (must specify `studentProfile.getName()`), **but** risks data inconsistency between two names and **loses `Is-a` polymorphism** (`Person*` cannot point to `TeachingAssistant`). |


---

# Task 03 – Constructors in Derived Classes and Member Classes

## Task 3.1 – Passing Values to Base Constructors & Virtual Base Rule

In a diamond hierarchy (`Person` $\leftarrow$ `Student`, `Person` $\leftarrow$ `Staff`, `{Student, Staff}` $\leftarrow$ `TeachingAssistant`), the **most-derived class (`TeachingAssistant`) must directly invoke the virtual base (`Person`) constructor** in its member initializer list:
```cpp
TeachingAssistant(...)
    : Person(n, i, city, pin),
      Student(name, id, city, pin, c, cCode, cr),
      Staff(name, id, city, pin, d),
      labSection(lab) {}
```
**Why the most-derived class must call the virtual base constructor:** If `Student` and `Staff` were allowed to initialize `Person`, `Person`'s constructor would either execute twice or face conflicting arguments from `Student` vs `Staff`. Therefore, the C++ standard mandates that the **most-derived class** constructs the single shared virtual base sub-object first, and intermediate classes' calls to `Person(...)` are automatically ignored when constructing a `TeachingAssistant`.

---

## Task 3.2 & 3.3 – Member Classes & Order of Construction / Destruction

* `Person` contains an `Address` member object (`addr`).
* `Student` contains a `Course` member object (`enrolledCourse`).

### Universal C++ Construction Order Rule:
1. **Virtual base classes** (top-down, left-to-right in depth-first inheritance graph).
2. **Non-virtual direct base classes** (left-to-right in the class declaration's base-specifier list).
3. **Non-static member objects** (in the **exact order they are declared inside the class body**, *not* the order written in the member initializer list).
4. **Derived class constructor body**.

**Destruction Rule:** Strictly the **exact reverse order** of construction (Derived body $
ightarrow$ Members in reverse declaration order $
ightarrow$ Non-virtual bases right-to-left $
ightarrow$ Virtual bases last).

---

## Task 3.4 – Experiment: Removing `virtual`

* **With `virtual`:** `Person` and `Address` are constructed **once** (`sizeof(TeachingAssistant)` = **288 bytes**).
* **Without `virtual`:** `Person` and `Address` are constructed **twice** (once inside `Student`, once inside `Staff`), and `sizeof(TeachingAssistant)` increases to **368 bytes** (+80 bytes of duplicated `Person` + `Address` data).

---

## Predict the Output

* **Predicted Output (Written Before Execution):**
  ```text
  Vehicle Engine Car | ~Car ~Engine ~Vehicle 
  ```
* **Actual Output:**
  ```text
  Vehicle Engine Car | ~Car ~Engine ~Vehicle 
  ```
* **Rule Explanation:** Base class `Vehicle` is constructed before member object `Engine e`, which is constructed before `Car`'s body prints `"Car "`. When `c` goes out of scope at `}`, destructors execute in exact reverse order: `~Car` $
ightarrow$ `~Engine` $
ightarrow$ `~Vehicle`.

---

## Complete the Code & `-Wall` Initializer Reorder Warning

* **Completed `TODO 1`:**
  ```cpp
  Student(const std::string& n, int a, long long r, const std::string& c, int p)
      : Person(n, a), regNo(r), home(c, p) {}
  ```
* **Actual Output:** `192311001 Kavya (19) Salem-636001` (Matches expected output).
* **Compiler Warning When Swapping Initializer List Order (`: home(c, p), regNo(r), Person(n, a)`):**
  ```text
  warning: 'Student::home' will be initialized after [-Wreorder]
  warning:   'long long int Student::regNo' [-Wreorder]
  warning:   when initialized here [-Wreorder]
  ```
  Even when listed out of order, C++ still initializes `Person` first, `regNo` second, and `home` third.

---

## Alternative Concept and Complexity Analysis

### Alternative Approach: Heap-Allocated `Address* home = new Address(...)` inside `Student`
If `Student` stores a raw pointer `Address* home;` instead of a value member `Address home;`:
1. **Extra Time:** Every `Student` construction incurs a dynamic heap allocation (`operator new`, $\mathcal{O}(1)$ amortized system allocator overhead + potential cache miss on pointer dereference), and destruction incurs `operator delete`.
2. **Extra Space:** `Student` stores an 8-byte `Address*` pointer **plus** heap allocator metadata overhead (typically 16 bytes per `new` block on 64-bit glibc).
3. **Rule of Three / Five Mandatory Boilerplate:**
   * **Constructor:** Allocates `home = new Address(c, p);`.
   * **Copy Constructor & Copy Assignment:** Must perform a **deep copy** (`home = new Address(*other.home);`) to prevent double-free crashes and shared-mutation bugs.
   * **Destructor:** Must explicitly call `delete home;` to prevent memory leaks.


---

# Task 04 – Pointers to Derived Classes: Forensics

## Task 4.1 – Locate the Three Defects

1. **Static Binding Instead of Dynamic Dispatch (`rate()` is Non-Virtual):** Because `Account::rate()` is not declared `virtual`, calling `a->rate()` through an `Account*` pointer resolves statically at compile time based on the pointer's static type (`Account*`), always returning `3.5%` even when pointing to a `SeniorAccount`.
2. **Undefined Behavior & 800-Byte Memory Leak (`~Account()` is Non-Virtual):** Deleting a derived `SeniorAccount` object via a base `Account*` pointer (`delete a;`) when `~Account()` is non-virtual invokes **undefined behavior** in C++: only `~Account()` runs, skipping `~SeniorAccount()`, leaking `new double[100]` (`800 bytes` per `SeniorAccount`).
3. **Object Slicing (`Account copy = SeniorAccount();`):** Assigning or copy-constructing a derived object by value into a base-type variable copies **only the `Account` sub-object**, permanently slicing off `SeniorAccount`'s derived members and overriding `vptr`. Calling `copy.rate()` or `copy.vrate()` on `copy` will always invoke `Account`'s version (`3.5%`).

---

## Task 4.2 – AddressSanitizer (`-fsanitize=address`) & Destructor Evidence

When compiled with `g++ -std=c++17 -g -fsanitize=address task04_defective_forensics.cpp`:
* Destructor log prints `[Dtor] ~Account() called` twice and **never** prints `[Dtor] ~SeniorAccount() called`.
* LeakSanitizer aborts with:
  ```text
  ERROR: LeakSanitizer: detected memory leaks
  Direct leak of 800 byte(s) in 1 object(s) allocated from:
      #1 in SeniorAccount::SeniorAccount()
  ```
* Furthermore, the temporary `SeniorAccount()` in `Account copy = SeniorAccount();` runs `~SeniorAccount()` immediately at the semicolon, proving `copy` is a pure base `Account` instance.

---

## Task 4.3 & 4.4 – Repaired Program & Prevention Rules

### Fixes Applied:
* Declared `virtual double rate() const` and `virtual ~Account()` in `Account`.
* Marked derived methods with `override` (`double rate() const override`, `~SeniorAccount() override`).
* Replaced raw owning pointers and value copies with `std::vector<std::unique_ptr<Account>>`.

### Two Prevention Rules:
1. **Polymorphic Base Destructor Rule:** Any class intended to be inherited from polymorphically (or having any `virtual` function) **must** declare a `public virtual` destructor (or a `protected` non-virtual destructor if base-pointer deletion is forbidden).
2. **Explicit `override` & Pointer/Reference Semantics Rule:** Always annotate overridden methods with `override` (so the compiler rejects accidental signature mismatches or non-virtual base functions) and store polymorphic objects by smart pointer (`std::unique_ptr<Base>`) or reference—never by value.

---

## Predict the Output

* **Predicted Output (Written Before Execution):**
  ```text
  3.5 4 3.5
  ```
* **Actual Output:**
  ```text
  3.5 4 3.5
  ```
* **Detailed Explanation:**
  1. `p->rate()` $
ightarrow$ **`3.5` (Compile-Time / Static Binding):** `Account::rate()` is non-virtual, so the compiler binds `p->rate()` directly to `Account::rate()` using `p`'s static type (`Account*`).
  2. `p->vrate()` $
ightarrow$ **`4` (Run-Time / Dynamic Dispatch):** `Account::vrate()` is `virtual`, and `p` points to a `SeniorAccount` object whose `vptr` points to `SeniorAccount`'s vtable, dispatching `SeniorAccount::vrate()` at runtime.
  3. `copy.vrate()` $
ightarrow$ **`3.5` (Object Slicing):** `Account copy = s;` constructs a standalone `Account` object by copying only the `Account` slice of `s`. `copy`'s `vptr` points to `Account`'s vtable, so `copy.vrate()` calls `Account::vrate()`.

---

## Alternative Concept and Complexity Analysis

### Alternative Approach: `std::vector<std::unique_ptr<Account>>` vs Raw `Account*` Array
* **Time Complexity:** Processing $n$ accounts takes $\mathcal{O}(n)$ time in both designs ($\mathcal{O}(1)$ vtable lookup per element). `std::unique_ptr` is a zero-overhead abstraction in optimized builds (`-O2`), compiling to identical machine instructions as raw `Account*`.
* **Space Complexity:** $\mathcal{O}(n)$ pointers (`sizeof(std::unique_ptr<Account>) == sizeof(Account*) == 8 bytes` on 64-bit systems), plus heap storage for each account.
* **Which of the Three Defects Can Still Occur?**
  * **Defect 1 (Non-virtual `rate()`)** and **Defect 3 (Object Slicing via `Account copy = *accounts[1];`)** can **still occur** if `virtual` is omitted or if an object is dereferenced and copied by value.
  * **Defect 2 (Forgetting `delete` / exception-unsafe leaks)** is eliminated by RAII, *provided* `~Account()` is still `virtual` (since `std::default_delete<Account>` executes `delete ptr;`, which still requires a virtual destructor on `Account`).


---

# Task 05 – Pointers to Objects and the `this` Pointer

## Task 5.1 & 5.3 – Four Ways to Reach an Object & `this` Pointer Mechanics

### Comparison of the Four Object Access Mechanisms

| Access Mechanism | Syntax Example | Safety | Object Lifetime | When to Use |
| :--- | :--- | :--- | :--- | :--- |
| **(a) Direct by Name** | `Cart c; c.add(...);` | **Highest** (never null, no dangling risk in scope) | Automatic storage duration (destroyed at block exit `}`) | Default choice for local, short-lived objects of known size. |
| **(b) Pointer to Existing Object** | `Cart* p = &c; p->add(...);` | **Moderate** (can be `nullptr` or dangle if `c` dies early) | Non-owning observer; tied to pointee `c`'s scope | Optional parameters, array elements, or reseatable references. |
| **(c) Reference (`Cart&`)** | `Cart& r = c; r.add(...);` | **High** (must be bound at init, cannot be null or reseated) | Alias to existing object `c` | Function parameters/returns (`Cart& add(...)`) to avoid copies safely. |
| **(d) Dynamic (`new` / `delete`)** | `Cart* p = new Cart; delete p;` | **Lowest** if raw (leak/double-free risk; prefer `unique_ptr`) | Dynamic heap duration until explicit `delete` | Polymorphic hierarchies or objects outliving the creating scope. |

### Why `this` is a Pointer & What `*this` Refers To
* **Why `this` is a pointer:** `this` originated in early C++ ("C with Classes") before references were added to the language, and because `this` is an implicit non-modifiable address (`Cart* const this` inside non-const member functions) passed via CPU register (`RDI` on x86-64 System V ABI) pointing to the invoking object.
* **What `*this` refers to:** Dereferencing `this` (`*this`) yields an **lvalue reference (`Cart&`) to the exact current object** on which the member function was invoked.

---

## Complete the Code & Return-by-Value Trap

* **Completed `TODO 1` & `TODO 2`:** Both `add()` and `setDiscount()` return `Cart&` via `return *this;`.
* **Actual Output:** `324.00` (Matches expected output).
* **What Would Happen If `add()` Returned `Cart` (by value) Instead of `Cart&`?**
  In `c.add("Rice", 60).add("Dal", 120).setDiscount(10);`:
  1. `c.add("Rice", 60)` adds `"Rice"` to `c`, then returns a **temporary copy** (`temp1`) of `c`.
  2. `.add("Dal", 120)` modifies `temp1` (NOT `c`!) and returns a second temporary copy (`temp2`).
  3. `.setDiscount(10)` sets the 10% discount on `temp2`, which is immediately destroyed at the semicolon!
  4. `c` would still contain **only `"Rice"` with `0%` discount**, and `p->total()` would output **`240.00` (`60 + 180`) instead of `324.00`**.

---

## Alternative Concept and Complexity Analysis

### Alternative Approach: `std::vector<Cart>` (By-Value Objects) vs Array of `Cart*` Pointers

| Metric | Array of `Cart*` Pointers (`Cart* activeCarts[]`) | `std::vector<Cart>` (Contiguous Objects) |
| :--- | :--- | :--- |
| **Time to Find Highest Total** | $\mathcal{O}(n)$ — $n$ pointer dereferences (`(*it)->total()`), potential cache misses if carts are scattered on heap. | $\mathcal{O}(n)$ — sequential contiguous memory scan with superior CPU cache locality. |
| **Space Complexity** | Array stores only $n$ pointers ($8n$ bytes), while `Cart` objects live elsewhere without copying. | Vector stores $n 	imes 	ext{sizeof}(	ext{Cart})$ bytes contiguously (up to $2	imes$ capacity buffer during growth) and copies/moves entire `Cart` objects on insertion. |
| **Pointer Invalidation on Growth** | **Never invalidates** external `Cart*` pointers when adding new pointers to a separate list. | **Dangling pointer hazard:** When `vec.push_back()` exceeds `vec.capacity()`, `std::vector` reallocates a larger buffer and destroys old elements, **invalidating all existing `Cart*` pointers and `Cart&` references** into the vector. |


---

# Task 06 – Design Challenge: Vehicle Rental Fleet

## Stage 1 – Interpret the Problem

### Entities, Attributes & Polymorphic Behaviour
* **Common (`Vehicle`):** `regNo`, `dailyRate`, `rented`, `bookedDays`. Behaviour: `rentOut()`, `returnVehicle()`, `computeCharge()` (varies per vehicle), `printDetails()`.
* **Non-Motorized (`Bicycle`):** `hasGear`. Charge = `days * (dailyRate + gearFee)`.
* **Motorized (`MotorVehicle`):** `energyLevelPercent`, `serviceDueDate` (`Date` member object).
* **Petrol Vehicles (`TwoWheeler`, `Car`):** `TwoWheeler` charges per litre of petrol; `Car` adds `seatingCapacity` and charges per litre + multi-seater insurance.
* **Electric Vehicles (`ElectricCar`, `ElectricBus`):** `batteryCapacityKWh`, `seatingCapacity`/`passengerCapacity`; charged **per kWh** instead of per litre.

### Testable Requirements
1. **REQ-1:** Renting an available vehicle for $d > 0$ days transitions its state to `RENTED`; renting an already-rented vehicle fails gracefully (`returns false`).
2. **REQ-2:** Returning a vehicle after $a$ actual days (when booked for $b$ days) computes `computeCharge(a, energyUsed)` plus a $50\% 	imes 	ext{dailyRate}$ late surcharge for $\max(0, a - b)$ overdue days, and resets state to `AVAILABLE`.
3. **REQ-3:** Adding a new vehicle type (`ElectricBus`) requires zero modifications to `Vehicle`, `MotorVehicle`, `Bicycle`, `TwoWheeler`, `Car`, `ElectricCar`, or `RentalFleet`.

---

## Stage 2 – Proposed Class Hierarchy (UML)

```text
                    +------------------------------------+
                    |           <<abstract>>             |
                    |              Vehicle               |
                    +------------------------------------+
                    | # regNo : string                   |
                    | # dailyRate : double               |
                    | # rented : bool                    |
                    | # bookedDays : int                 |
                    +------------------------------------+
                    | + virtual ~Vehicle()               |
                    | + rentOut(days) : bool             |
                    | + returnVehicle(days, energy)      |
                    | + virtual category() : string = 0  |
                    | + virtual computeCharge(...) = 0   |
                    +------------------------------------+
                              ^                ^
                              |                |
               +--------------+                +------------------+
               | (public)                                         | (public)
+-----------------------------+                    +------------------------------+
|           Bicycle           |                    |         MotorVehicle         |<>---- [Date]
+-----------------------------+                    +------------------------------+   (Has-a composition)
| - hasGear : bool            |                    | # energyLevelPercent : double|
+-----------------------------+                    | # serviceDueDate : Date      |
| + computeCharge(...)        |                    +------------------------------+
+-----------------------------+                       ^          ^          ^
                                                      |          |          |
                                         +------------+          |          +-------------+
                                         |                       |                        |
                              +--------------------+  +--------------------+  +--------------------+
                              |     TwoWheeler     |  |        Car         |  |    ElectricCar     |
                              +--------------------+  +--------------------+  +--------------------+
                              | + computeCharge()  |  | # seatingCapacity  |  | - batteryCapacity  |
                              +--------------------+  | + computeCharge()  |  | + computeCharge()  |
                                                      +--------------------+  +--------------------+
```

---

## Stage 3 – Critical Design Decisions

1. **Abstract Base Class (`Vehicle`):** Declares `category()` and `computeCharge()` as **pure virtual (`= 0`)** so no unspecialized `Vehicle` can be instantiated, while hoisting shared state (`regNo`, `dailyRate`, `rented`) and the Template Method `returnVehicle()` into `Vehicle` to avoid duplicate late-fee logic.
2. **Intermediate `MotorVehicle` Class + `Date` Composition:** Prevents duplicating `energyLevelPercent` and `serviceDueDate` across `TwoWheeler`, `Car`, `ElectricCar`, and `ElectricBus` without forcing `Bicycle` to carry engine/fuel fields.
3. **Virtual Destructor & Fleet Storage:** `RentalFleet` stores `Vehicle* fleet[MAX_VEHICLES]` (an array of base-class pointers) and deletes them via `virtual ~Vehicle() = default;`, guaranteeing leak-free polymorphic destruction.

---

## Stage 4 – SIMATS Standard Algorithmic Pseudocode (Rent & Return)

```text
ALGORITHM RentVehicle(fleet, count, targetReg, days)
INPUT:  fleet[0..count-1] (array of Vehicle pointers), targetReg (string), days (integer)
OUTPUT: boolean status (TRUE if rented, FALSE otherwise)

BEGIN
    IF days <= 0 THEN
        RETURN FALSE
    END IF

    FOR i <- 0 TO count - 1 DO
        IF fleet[i]->getRegNo() = targetReg THEN
            IF fleet[i]->isRented() = TRUE THEN
                RETURN FALSE
            END IF
            fleet[i]->rented <- TRUE
            fleet[i]->bookedDays <- days
            RETURN TRUE
        END IF
    END FOR

    RETURN FALSE
END
```

```text
ALGORITHM ReturnVehicle(vehiclePtr, actualDays, energyUsed)
INPUT:  vehiclePtr (Vehicle pointer), actualDays (integer), energyUsed (real)
OUTPUT: totalBill (real; -1.0 if vehicle was not rented)

BEGIN
    IF vehiclePtr = NULL OR vehiclePtr->isRented() = FALSE THEN
        RETURN -1.0
    END IF

    baseCharge <- vehiclePtr->computeCharge(actualDays, energyUsed)   // Dynamic dispatch

    IF actualDays > vehiclePtr->bookedDays THEN
        extraDays <- actualDays - vehiclePtr->bookedDays
    ELSE
        extraDays <- 0
    END IF

    latePenalty <- extraDays * (vehiclePtr->dailyRate * 0.50)
    totalBill   <- baseCharge + latePenalty

    vehiclePtr->rented     <- FALSE
    vehiclePtr->bookedDays <- 0

    RETURN totalBill
END
```

---

## Alternative Concept and Complexity Analysis

### Alternative Approach: Single Monolithic `Vehicle` Class with `enum VehicleType` and `switch` in `computeCharge()`

| Metric | Abstract Base Class + Virtual Functions | Single `Vehicle` Class + `enum` + `switch` |
| :--- | :--- | :--- |
| **Time per `computeCharge()` Call** | $\mathcal{O}(1)$ — single indirect vtable lookup (`vptr[slot]`). | $\mathcal{O}(1)$ — `switch` jump-table or branch chain. |
| **Space per Object** | Compact: `Bicycle` stores only `Vehicle` fields + `bool hasGear` + 1 `vptr` (8 B); `Bicycle` carries **zero** battery/fuel/seat fields. | Bloated: Every `Bicycle` object must allocate unused memory for `energyLevelPercent`, `serviceDueDate`, `seatingCapacity`, and `batteryCapacityKWh` (+24–32 wasted bytes per object). |
| **Classes Changed When Adding `ElectricBus`** | **0 existing classes modified** (only 1 new subclass `ElectricBus` added — strictly Open/Closed). | **Modifies `Vehicle` header & implementation** (`enum VehicleType`, constructors, new fields, `switch` inside `computeCharge()` and `printDetails()`), forcing full recompilation and retesting of all vehicle types. |


---
