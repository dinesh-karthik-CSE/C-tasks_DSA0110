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

**Destruction Rule:** Strictly the **exact reverse order** of construction (Derived body $ightarrow$ Members in reverse declaration order $ightarrow$ Non-virtual bases right-to-left $ightarrow$ Virtual bases last).

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
* **Rule Explanation:** Base class `Vehicle` is constructed before member object `Engine e`, which is constructed before `Car`'s body prints `"Car "`. When `c` goes out of scope at `}`, destructors execute in exact reverse order: `~Car` $ightarrow$ `~Engine` $ightarrow$ `~Vehicle`.

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
