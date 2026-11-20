# DSA01 – Object Oriented Programming with C++ (Assignment 5 – CO5)

## Student Details
* **Student Name:** `<Your Name>`
* **Register Number:** `<Your Register Number>`
* **Course Code & Title:** `DSA01` – Object Oriented Programming with C++
* **Course Outcome:** `CO5` – Reliable Software with Runtime Polymorphism and Exceptions
* **Compiler & Version:** `g++ (Debian 12.2.0-14+deb12u1) 12.2.0`
* **Standard Compilation Flags:** `g++ -std=c++17 -Wall -Wextra <source.cpp> -o <exe>` (plus `-fsanitize=address` and `-O2` for forensics & benchmarks)

---

## Repository Structure

```text
DSA01-<RegisterNumber>-Assignment5/
├── README.md                          # Repository overview, build commands & task summaries
├── Assignment5_Complete_Report.md     # Full K5 report (Part A Design + Part B Benchmarks & Forensics)
├── PartA_Design/
│   ├── README_PartA_Design.md         # Tasks A1–A5: UML, vtables, SIMATS pseudocode, exception hierarchy & predictions
│   ├── taskA2_predict_output.cpp
│   ├── taskA2_nonvirtual_dtor_asan.cpp
│   ├── taskA5_predict_stack_unwinding.cpp
│   └── output_partA_predictions.png
├── TaskB1/
│   ├── README_TaskB1.md               # Hand-verified settlement matrix, virtual/dtor proofs & template analysis
│   ├── taskB1_complete_code.cpp
│   ├── taskB1_hospital_matrix.cpp
│   ├── taskB1_asan_no_virtual_dtor.cpp
│   ├── output_taskB1_matrix.png
│   └── output_taskB1_asan.png
├── TaskB2/
│   ├── README_TaskB2.md               # 6 engineered error tests, stack unwinding, throw vs throw e, std::optional analysis
│   ├── taskB2_predict_output.cpp
│   ├── taskB2_complete_code.cpp
│   ├── taskB2_error_conditions.cpp
│   ├── output_taskB2_predict_complete.png
│   └── output_taskB2_stress_asan.png
├── TaskB3/
│   ├── README_TaskB3.md               # -O2 benchmarks (1K, 1M, 10M bills) & exception cost across 0%, 1%, 50% error rates
│   ├── taskB3_benchmark.cpp
│   └── output_taskB3_benchmark.png
└── TaskB4/
    ├── README_TaskB4.md               # Practical extension (TeleMedicineBill + CorporateTieUp) & production decision
    ├── taskB4_production_extension.cpp
    └── output_taskB4_asan_clean.png
```

---

## Compilation, Execution & Task Summaries

### Part A – Design & Output Predictions (`PartA_Design/`)
```bash
cd PartA_Design
g++ -std=c++17 -Wall -Wextra taskA2_predict_output.cpp -o a2p && ./a2p
g++ -std=c++17 -g -fsanitize=address taskA2_nonvirtual_dtor_asan.cpp -o a2_asan && ./a2_asan
g++ -std=c++17 -Wall -Wextra taskA5_predict_stack_unwinding.cpp -o a5p && ./a5p
```
* **Summary:** Designed `Bill` and `Payer` abstract hierarchies with UML & vtable layouts, SIMATS pseudocode (`ComputeDailyBillingTotal`), `std::exception` hierarchy, and verified `500 ~OPBill ~Bill` (A2) and `~f ~main caught 42` (A5).

### Task B1 – Implement & Validate Polymorphic Billing (`TaskB1/`)
```bash
cd TaskB1
g++ -std=c++17 -Wall -Wextra taskB1_complete_code.cpp -o b1c && ./b1c
g++ -std=c++17 -Wall -Wextra taskB1_hospital_matrix.cpp -o b1m && ./b1m
g++ -std=c++17 -g -fsanitize=address taskB1_asan_no_virtual_dtor.cpp -o b1_asan && ./b1_asan
```
* **Summary:** Completed `Bill` TODOs (`49200`), validated all 12 `Bill x Payer` combinations against hand calculations, and reproduced AddressSanitizer `new-delete-type-mismatch` when `~Bill()` is non-virtual.

### Task B2 – Engineer Error Conditions (`TaskB2/`)
```bash
cd TaskB2
g++ -std=c++17 -Wall -Wextra taskB2_predict_output.cpp -o b2p && ./b2p
g++ -std=c++17 -Wall -Wextra taskB2_complete_code.cpp -o b2c && ./b2c
g++ -std=c++17 -Wall -Wextra -fsanitize=address taskB2_error_conditions.cpp -o b2st && ASAN_OPTIONS=allocator_may_return_null=1 ./b2st
```
* **Summary:** Engineered and recovered from all 6 failure conditions (negative fees, exceeded cover, unknown package code, `std::stod` invalid input, `std::bad_alloc`, and `catch (...)`) with RAII destructor logs proving zero leaks.

### Task B3 – Virtual Functions vs `switch` vs Exceptions (`TaskB3/`)
```bash
cd TaskB3
g++ -std=c++17 -O2 -Wall -Wextra taskB3_benchmark.cpp -o b3 && ./b3
```
* **Summary:** Benchmarked `virtual` vs `switch` vs `std::variant` up to 10,000,000 bills (`291.2 ms` vs `208.2 ms` vs `169.5 ms`) and measured exception overhead across `0%` (`8.4 ms`), `1%` (`31.3 ms`), and `50%` (`1159.6 ms`) error rates.

### Task B4 – Final Engineering Decision (`TaskB4/`)
```bash
cd TaskB4
g++ -std=c++17 -Wall -Wextra -fsanitize=address taskB4_production_extension.cpp -o b4 && ./b4
```
* **Summary:** Added `TeleMedicineBill` and `CorporateTieUp` without modifying existing classes, verified clean ASan exit under thrown exceptions, and justified the production architecture for 200,000 bills/day across 15 hospitals.

---

## Progressive Git Commit Commands
```bash
git init && git checkout -b main
git add README.md && git commit -m "chore: initialize Assignment 5 repository and root README"
git add PartA_Design && git commit -m "docs(partA): add UML, vtables, SIMATS pseudocode, exception design and predictions"
git add TaskB1 && git commit -m "feat(taskB1): implement polymorphic Bill/Payer matrix and ASan virtual destructor proof"
git add TaskB2 && git commit -m "feat(taskB2): engineer 6 error scenarios with RAII stack unwinding and bare rethrow"
git add TaskB3 && git commit -m "perf(taskB3): benchmark virtual vs switch vs variant and exception overhead at -O2"
git add TaskB4 Assignment5_Complete_Report.md && git commit -m "feat(taskB4): extend with TeleMedicineBill and CorporateTieUp and finalize report"
```
