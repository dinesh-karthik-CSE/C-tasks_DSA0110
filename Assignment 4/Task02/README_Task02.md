# Task 02 – From Requirement to Hierarchy (University People Management)

## Task 2.1 – Relationship Classification & Inheritance Types

| Entity / Relationship | Classification | Type of Inheritance / Composition | Justification |
| :--- | :--- | :--- | :--- |
| `Student` $ightarrow$ `Person` | **Is-a** | Hierarchical / Virtual Single Inheritance | Every student is a person with common attributes (`name`, `id`, `phone`). |
| `Staff` $ightarrow$ `Person` | **Is-a** | Hierarchical / Virtual Single Inheritance | Every staff member is also a person sharing identity attributes. |
| `TeachingAssistant` $ightarrow$ `Student`, `Staff` | **Is-a** (both) | **Multiple + Hybrid (Diamond)** Inheritance | A TA is simultaneously a student and a staff member. |
| `ResearchScholar` $ightarrow$ `Student` | **Is-a** | **Multilevel** Inheritance (`Person` $ightarrow$ `Student` $ightarrow$ `ResearchScholar`) | A research scholar is a specialized student. |
| `ResearchScholar` $ightarrow$ `Fellowship` | **Has-a** | **Composition (Member Class)** | A fellowship (grant agency, stipend amount) is an external award held by the scholar, not a person subtype. |

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

## Task 2.4 – Access Visibility Table (`Person` $ightarrow$ `Student`)

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
