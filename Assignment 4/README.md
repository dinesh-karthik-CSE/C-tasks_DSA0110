# DSA01 – Object Oriented Programming with C++ (Assignment 4 – CO4)

## Student Details
* **Student Name:** `<Your Name>`
* **Register Number:** `<Your Register Number>`
* **Course Code & Title:** `DSA01` – Object Oriented Programming with C++
* **Course Outcome:** `CO4` – Designing Reusable Class Hierarchies (Inheritance, Virtual Base Classes, Abstract Classes, Pointers & Polymorphism)
* **Compiler & Version:** `g++ (Debian 12.2.0-14+deb12u1) 12.2.0`
* **Standard Compilation Flags:** `g++ -std=c++17 -Wall -Wextra <source_file.cpp> -o <executable>`

---

## Repository Structure

```text
DSA01-<RegisterNumber>-Assignment4/
├── README.md                          # Repository overview, compilation instructions & task summaries
├── Assignment4_Complete_Report.md     # Consolidated K5 evaluation report (UML, predictions, traces, comparisons)
├── Task01/
│   ├── task01_payroll_hierarchy.cpp
│   ├── README_Task01.md
│   └── output_task01.png
├── Task02/
│   ├── task02_university_hierarchy.cpp
│   ├── task02_predict_virtual.cpp
│   ├── task02_predict_nonvirtual.cpp
│   ├── README_Task02.md
│   ├── output_task02_hierarchy.png
│   └── output_task02_predict.png
├── Task03/
│   ├── task03_construction_trace.cpp
│   ├── task03_nonvirtual_experiment.cpp
│   ├── task03_predict_output.cpp
│   ├── task03_complete_code.cpp
│   ├── task03_reorder_warning.cpp
│   ├── README_Task03.md
│   ├── output_task03_trace.png
│   ├── output_task03_experiment.png
│   └── output_task03_complete_and_predict.png
├── Task04/
│   ├── task04_defective_forensics.cpp
│   ├── task04_repaired.cpp
│   ├── task04_predict_output.cpp
│   ├── README_Task04.md
│   ├── output_task04_asan_leak.png
│   └── output_task04_repaired_and_predict.png
├── Task05/
│   ├── task05_complete_code.cpp
│   ├── task05_four_ways_and_search.cpp
│   ├── README_Task05.md
│   └── output_task05.png
└── Task06/
    ├── task06_vehicle_rental_fleet.cpp
    ├── README_Task06.md
    └── output_task06.png
```

---

## Compilation & Execution Commands

### Task 01 – Class Design Review (Payroll Hierarchy)
```bash
cd Task01
g++ -std=c++17 -Wall -Wextra task01_payroll_hierarchy.cpp -o task01 && ./task01
```
* **Result Summary:** Refactored duplicated `Professor`, `LabAssistant`, and `GuestFaculty` classes into a polymorphic `Staff` $ightarrow$ `SalariedStaff` hierarchy with `DA_RATE = 0.42`, processing 4 heterogeneous staff roles (including a newly added `VisitingFellow`) in a single loop totaling `Rs. 281300.00`.

### Task 02 – University People Management (Diamond Inheritance)
```bash
cd Task02
g++ -std=c++17 -Wall -Wextra task02_university_hierarchy.cpp -o task02 && ./task02
g++ -std=c++17 -Wall -Wextra task02_predict_virtual.cpp -o p_virt && ./p_virt
g++ -std=c++17 -Wall -Wextra task02_predict_nonvirtual.cpp -o p_nonvirt && ./p_nonvirt
```
* **Result Summary:** Resolved the diamond ambiguity in `TeachingAssistant` using `virtual public Person`, verified single construction (`Person Student Staff TA` vs `Person Student Person Staff TA`), and modeled `ResearchScholar` using multilevel inheritance + `Fellowship` composition.

### Task 03 – Constructors in Derived Classes and Member Classes
```bash
cd Task03
g++ -std=c++17 -Wall -Wextra task03_construction_trace.cpp -o trace && ./trace
g++ -std=c++17 -Wall -Wextra task03_nonvirtual_experiment.cpp -o exp && ./exp
g++ -std=c++17 -Wall -Wextra task03_predict_output.cpp -o pred3 && ./pred3
g++ -std=c++17 -Wall -Wextra task03_complete_code.cpp -o comp3 && ./comp3
g++ -std=c++17 -Wall -Wextra task03_reorder_warning.cpp -o warn3
```
* **Result Summary:** Traced construction/destruction order across virtual bases, non-virtual bases, and member classes (`Address`, `Course`), confirmed `sizeof(TeachingAssistant)` increases from `216` to `288` bytes without `virtual`, completed `TODO 1` (`192311001 Kavya (19) Salem-636001`), and captured `-Wreorder` diagnostics.

### Task 04 – Pointers to Derived Classes: Forensics
```bash
cd Task04
g++ -std=c++17 -g -fsanitize=address task04_defective_forensics.cpp -o def4 && ./def4
g++ -std=c++17 -Wall -Wextra -fsanitize=address task04_repaired.cpp -o rep4 && ./rep4
g++ -std=c++17 -Wall -Wextra task04_predict_output.cpp -o pred4 && ./pred4
```
* **Result Summary:** Reproduced AddressSanitizer `new-delete-type-mismatch` and memory leak on `SeniorAccount`, diagnosed static binding and object slicing (`3.5 4 3.5`), and repaired the hierarchy using `virtual` methods, `override`, `virtual ~Account()`, and `std::unique_ptr<Account>`.

### Task 05 – Pointers to Objects and the `this` Pointer
```bash
cd Task05
g++ -std=c++17 -Wall -Wextra task05_complete_code.cpp -o comp5 && ./comp5
g++ -std=c++17 -Wall -Wextra task05_four_ways_and_search.cpp -o ways5 && ./ways5
```
* **Result Summary:** Completed `Cart::add` and `Cart::setDiscount` returning `*this` (`324.00`), demonstrated all 4 object access mechanisms (direct, pointer `->`, reference `&`, dynamic `new`/`delete`), and found the highest-value cart (`Rs. 855.00`) via pointer arithmetic over `Cart* activeCarts[]`.

### Task 06 – Design Challenge: Vehicle Rental Fleet
```bash
cd Task06
g++ -std=c++17 -Wall -Wextra task06_vehicle_rental_fleet.cpp -o fleet && ./fleet
```
* **Result Summary:** Designed and implemented an extensible `Vehicle` $ightarrow$ `MotorVehicle` polymorphic hierarchy with `Date` composition, demonstrated renting, late-return penalty calculation (`Rs. 12600.00` for 2-day late `ElectricCar`), and added `ElectricBus` without altering any existing class.

---

## Progressive Git Commit Guide (Run Locally Before Pushing)
To satisfy the progressive commit requirement (at least 1 commit per task):
```bash
git init
git checkout -b main
git add README.md
git commit -m "chore: initialize repository and add root README"

git add Task01 && git commit -m "feat(task01): diagnose payroll defects and implement polymorphic Staff hierarchy"
git add Task02 && git commit -m "feat(task02): resolve diamond inheritance with virtual base Person and access table"
git add Task03 && git commit -m "feat(task03): trace base/member construction order and complete Student initializer list"
git add Task04 && git commit -m "fix(task04): capture ASan leak forensics and repair Account polymorphism with virtual dtor"
git add Task05 && git commit -m "feat(task05): implement Cart method chaining with this pointer and pointer-array search"
git add Task06 Assignment4_Complete_Report.md && git commit -m "feat(task06): implement Vehicle Rental Fleet design challenge and complete report"
```
