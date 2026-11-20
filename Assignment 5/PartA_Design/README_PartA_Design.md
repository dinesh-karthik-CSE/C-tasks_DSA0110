# PART A – DESIGN: HOSPITAL BILLING AND INSURANCE CLAIMS (25 Marks)

---

## TASK A1 – DESIGN THE POLYMORPHIC BILL HIERARCHY (7 Marks)

### A1.1 – Common Interface & Shared State
* **Common Data (`protected` in `Bill`):** `billId` (`string`), `patientName` (`string`), `packageCode` (`string`).
* **Common Interface (`public` in `Bill`):**
  * `virtual ~Bill() = default;` (polymorphic cleanup)
  * `virtual double total() const = 0;` (pure virtual — computes total charges)
  * `virtual std::string type() const = 0;` (pure virtual — returns category label)
  * `virtual void print() const;` (virtual — prints common header and polymorphic `total()`)

### A1.2 – UML Class Diagram
```text
                    +----------------------------------------+
                    |             <<abstract>>               |
                    |                  Bill                  |
                    +----------------------------------------+
                    | # billId : string                      |
                    | # patientName : string                 |
                    | # packageCode : string                 |
                    +----------------------------------------+
                    | + virtual ~Bill()                      |
                    | + virtual total() : double = 0         |
                    | + virtual type() : string = 0          |
                    | + virtual print() : void               |
                    +----------------------------------------+
                       ^          ^            ^           ^
                       |          |            |           |
        +--------------+          |            |           +----------------+
        | (public)                | (public)   | (public)                   | (public)
+---------------+       +---------------+   +---------------+       +-----------------+
|OutPatientBill |       | InPatientBill |   |  DayCareBill  |       |  EmergencyBill  |
+---------------+       +---------------+   +---------------+       +-----------------+
| - consultFee  |       | - days        |   | - packageFee  |       | - baseCharge    |
| - testCharges |       | - roomPerDay  |   +---------------+       | - hours         |
+---------------+       | - procCharges |   | + total()     |       | - hourlyRate    |
| + total()     |       | - medCharges  |   | + type()      |       +-----------------+
| + type()      |       +---------------+   +---------------+       | + total()       |
+---------------+       | + total()     |                           | + type()        |
                        | + type()      |                           +-----------------+
                        +---------------+
```

### A1.3 – Process All Bills Together (SIMATS Standard Algorithmic Pseudocode)
```text
ALGORITHM ComputeDailyBillingTotal(bills, n)
INPUT:  bills[0..n-1] (array of pointers to Bill), n (integer)
OUTPUT: dailyTotal (real)

BEGIN
    dailyTotal <- 0.0

    FOR i <- 0 TO n - 1 DO
        IF bills[i] != NULL THEN
            dailyTotal <- dailyTotal + bills[i]->total()   // Dynamic dispatch via vtable
        END IF
    END FOR

    RETURN dailyTotal
END
```

### Alternative Concept & Complexity Analysis (`Bill` Struct + `type` Tag + `switch`)
* **Time to Total $n$ Bills:** Both designs execute in $\mathcal{O}(n)$ time ($\mathcal{O}(1)$ per bill: one indirect `vptr` call vs one `switch` jump-table/branch).
* **Space per Bill:** With virtual inheritance, each derived bill holds only its own fields plus one 8-byte `vptr` (e.g., `DayCareBill` holds only `packageFee`). A monolithic `Bill` struct must carry **all fields of every bill type simultaneously** (`consultFee`, `testCharges`, `days`, `roomPerDay`, `procCharges`, `medCharges`, `packageFee`, `baseCharge`, `hours`, `hourlyRate`, plus `enum type`), wasting 48–64 bytes per `OutPatientBill` or `DayCareBill`.
* **Code Changed When Adding a New Bill Type:** With `Bill` hierarchy, **0 existing classes change** (add 1 subclass). With `struct Bill` + `switch`, you must modify `enum BillType`, `struct Bill` fields, constructors, `switch` inside `total()`, and `switch` inside `type()` / `print()`.

---

## TASK A2 – UNDERSTAND RUNTIME BINDING (5 Marks)

### A2.1 – Compile-Time (Early) vs Run-Time (Late) Binding
* **Compile-Time (Early) Binding:** For non-virtual member functions (e.g., `b->show()`), the compiler resolves the exact function address (`Bill::show`) statically using the pointer's declared type (`Bill*`).
* **Run-Time (Late) Binding:** For `virtual` member functions (e.g., `b->total()`), the compiler emits machine code that dereferences the object's hidden `vptr` at runtime, looks up the function slot in the dynamic type's `vtable` (`OPBill`), and calls `OPBill::total()`.

### A2.2 – Virtual Table (`vtable`) Layout & Call Resolution
```text
Object in Heap (b)                OPBill vtable (Read-Only Memory)
+---------------------+           +-----------------------------------+
| vptr ---------------+---------->| [0] &OPBill::~OPBill()            |
| (Bill/OPBill data)  |           | [1] &OPBill::total()              |
+---------------------+           +-----------------------------------+

Bill vtable (Abstract)            InPatientBill vtable
+-----------------------------+   +-----------------------------------+
| [0] &Bill::~Bill()          |   | [0] &InPatientBill::~InPatientBill|
| [1] __cxa_pure_virtual      |   | [1] &InPatientBill::total()       |
+-----------------------------+   +-----------------------------------+
```
When `b->total()` executes, the CPU reads `b->vptr`, indexes slot `[1]` (`vptr[1]`), and jumps directly to `OPBill::total()`.

### A2.3 – Why `Bill` Must Have a Virtual Destructor
Deleting a derived object through a base pointer (`delete b;`) when `~Bill()` is non-virtual causes **undefined behavior** in C++: the derived destructor (`~OPBill()`) is bypassed, leaking derived resources (heap buffers, file handles) and passing the wrong object size to `operator delete`.

### Predict the Output (Task A2)
* **Predicted Output (Written Before Running):**
  ```text
  500 ~OPBill ~Bill 
  ```
* **Actual Output:** `500 ~OPBill ~Bill `
* **How `show()` Reached `OPBill::total()`:** Inside `Bill::show()`, the implicit call `total()` is `this->total()`. Because `this` points to an `OPBill` instance at runtime and `total()` is `virtual`, `this->vptr` dispatches dynamically to `OPBill::total()` (`500`).
* **Removing `virtual` from `~Bill`:**
  * **Predicted Output:** `500 ~Bill ` (`~OPBill` is skipped!) and under `-fsanitize=address`, AddressSanitizer aborts with `ERROR: AddressSanitizer: new-delete-type-mismatch` (allocated 8 bytes for `OPBill`, deallocated 1 byte as `Bill`).
  * **Verified Output:** Matches prediction (`new-delete-type-mismatch` confirmed by ASan).

### Alternative Concept & Complexity Analysis (`dynamic_cast` Chain vs Virtual `total()`)
* **Time per Bill:** Virtual dispatch performs **1 indirect jump ($\mathcal{O}(1)$)** regardless of the number of bill types $k$. A `dynamic_cast` chain (`if (auto* p = dynamic_cast<OPBill*>(b)) ... else if (...)`) traverses RTTI `type_info` graphs up to **$k$ times per bill ($\mathcal{O}(k)$)**.
* **Space per Object:** Virtual classes require one 8-byte `vptr` per object (which `dynamic_cast` also requires anyway because `dynamic_cast` only works on polymorphic types with a vtable!).

---

## TASK A3 – DESIGN WITH ABSTRACT CLASSES (5 Marks)

### A3.1 – Abstract `Payer` Interface & Derived Classes
```text
               +------------------------------------------+
               |              <<abstract>>                |
               |                 Payer                    |
               +------------------------------------------+
               | # payerName : string                     |
               +------------------------------------------+
               | + virtual ~Payer()                       |
               | + virtual amountCovered(const Bill&) = 0 |
               | + virtual payerType() : string = 0       |
               +------------------------------------------+
                   ^                 ^                  ^
                   |                 |                  |
        +----------+                 |                  +--------------+
        |                            |                                 |
+---------------+        +-----------------------+        +------------------------+
|    SelfPay    |        |       Insurance       |        |    GovernmentScheme    |
+---------------+        +-----------------------+        +------------------------+
|               |        | - coverLimit : double |        | - fixedPackageRate     |
+---------------+        | - coPayPct : double   |        +------------------------+
| + amountCov() |        +-----------------------+        | + amountCovered(...)   |
+---------------+        | + amountCovered(...)  |        +------------------------+
                         +-----------------------+
```
* **`SelfPay::amountCovered(bill)`:** Returns `0.0` (patient pays 100% out of pocket).
* **`Insurance::amountCovered(bill)`:** Throws `ClaimError` if `bill.total() > coverLimit` (or caps per policy rule); otherwise pays `bill.total() * (1.0 - coPayPct / 100.0)`.
* **`GovernmentScheme::amountCovered(bill)`:** Looks up `bill.getPackageCode()` in approved scheme packages; throws `UnknownPackageError` if absent, otherwise covers `std::min(bill.total(), fixedPackageRate)`.

### A3.2 – Extensibility (`CorporateTieUp` Payer)
Adding `class CorporateTieUp : public Payer` requires only overriding `amountCovered(const Bill&)` (e.g., 90% employer subsidy up to corporate cap) and `payerType()`. Neither `Bill` nor any existing `Payer` subclass is modified.

### Alternative Concept & Complexity Analysis (`std::function` Policy vs Abstract `Payer`)
* **Time per Call:** Both execute in $\mathcal{O}(1)$ time (`Payer` uses a vtable jump; `std::function` uses an internal manager/invoker function pointer).
* **Space per Payer:** An abstract `Payer` subclass stores `vptr` (8 B) + exact member fields (`coverLimit`, `coPayPct` = 24 B total). `std::function<double(const Bill&)>` typically uses a fixed 32-byte buffer (Small Object Optimization) and **allocates on the heap** if captured state (`coverLimit`, `coPayPct`, `policyId` string) exceeds the 16-byte SOO threshold, scattering state inside untyped lambda closures rather than inspecting/querying named class members.

---

## TASK A4 – DESIGN THE EXCEPTION STRATEGY (4 Marks)

### A4.1 – Error List & Exception Hierarchy (`std::exception`)
1. **Negative Charges / Invalid Inputs** (`BillingValidationError`)
2. **Claim Above Policy Cover** (`ClaimLimitExceededError`)
3. **Unknown Government Package Code** (`UnknownPackageError`)
4. **Missing Patient Record** (`PatientNotFoundError`)
5. **Memory Allocation Failure** (`std::bad_alloc` — standard library exception)

```text
std::exception
 ├── std::bad_alloc                  (Heap exhaustion)
 └── HospitalException               (Base domain exception carrying errorCode & context)
      ├── BillingValidationError     (Negative fees, days <= 0, malformed numeric text)
      ├── PatientNotFoundError       (Missing patient ID in registry)
      └── ClaimError                 (Insurance & scheme rejection base)
           ├── ClaimLimitExceededError (Billed amount > policy coverLimit)
           └── UnknownPackageError     (Unregistered government package code)
```

### A4.2 – Exception Specifications (`noexcept`) & Non-Throwing Destructors
* **May Throw:** Constructors validating inputs (`OutPatientBill`, `InPatientBill`), `settleClaim()`, `parseBillInput()` (`std::stod`), and dynamic allocations (`new`).
* **Marked `noexcept`:** Getters (`getBillId() const noexcept`, `getPatientName() const noexcept`), state queries (`isSettled() const noexcept`), move operations, and all destructors (`~Bill() noexcept`).
* **Why Destructors Must Never Throw:** During stack unwinding for an active exception, C++ invokes destructors of all local objects. If a destructor throws a second exception while another exception is already active (`std::uncaught_exceptions() > 0`), the C++ runtime immediately invokes **`std::terminate()`**, aborting the program without cleanup.

### Alternative Concept & Complexity Analysis (Return Codes `enum Status` vs Exceptions)
* **Normal Path Time:** Exceptions have **zero runtime branching overhead** on the happy path (modern compilers use zero-cost DWARF unwind tables), whereas return codes require `if (status != Status::OK) return status;` checks at **every single call level** ($\mathcal{O}(d)$ branches for call depth $d$).
* **Error Path Time:** Returning an error code is $\mathcal{O}(1)$ stack return (~1–5 ns), whereas throwing an exception unwinds frames and allocates exception objects (~1,000–5,000 ns).
* **Space & Code Overhead:** Return codes require returning `std::pair<Status, double>` or out-parameters and cluttering every intermediate caller with manual error propagation boilerplate.

---

## TASK A5 – DESIGN RECOVERY AND RETHROWING (4 Marks)

### A5.1 – Three-Tier Catch, Log & Rethrow Architecture
1. **Domain Layer (`Bill` / `Payer`):** Detects invariant violations and `throw`s specific domain exceptions (`BillingValidationError`, `ClaimLimitExceededError`).
2. **Service / Audit Layer (`BillingService::processClaim`):** Catches `const ClaimError& e`, writes a structured audit entry (`[AUDIT] Claim rejected: ...`), and executes **`throw;`** (bare rethrow) to preserve the exact dynamic exception type and original exception object without slicing.
3. **Top-Level Presentation / Batch Controller (`main`):** Catches domain exceptions (`const HospitalException&`), `const std::bad_alloc&`, and a final `catch (...)` safety net so one bad claim is rejected cleanly while the remaining daily bills continue processing without data loss.

### A5.2 – Stack Unwinding & RAII Resource Safety
When an exception is thrown, C++ unwinds the call stack backward from the `throw` point to the matching `catch` block, invoking the destructor of every fully constructed automatic (stack) object in reverse order. Wrapping heap memory in `std::unique_ptr<Bill>` / `std::vector` and files in `std::ofstream` (RAII) guarantees deterministic cleanup even when exceptions bypass normal `return` statements.

### Predict the Output (Task A5)
* **Predicted Output (Written Before Running):**
  ```text
  ~f ~main caught 42
  ```
* **Actual Output:** `~f ~main caught 42`
* **Explanation:** `throw 42;` inside `f()` immediately aborts `f()` and unwinds its stack frame, calling `g.~Guard()` (`~f `). Control jumps out of the `try` block in `main()`—skipping `std::cout << "not printed ";`—and unwinds `m.~Guard()` (`~main `) before entering `catch (int x)` (`caught 42`).

### Alternative Concept & Complexity Analysis (Manual `delete` in Every `catch` vs RAII)
* **Time Complexity:** Identical $\mathcal{O}(1)$ destructor execution during unwinding.
* **Code Size & Leak Risk:** Manual cleanup forces duplicating `delete ptr;` before **every** normal `return` AND inside **every** `catch` block (`catch (ClaimError&)`, `catch (std::exception&)`, `catch (...)`). If there are $E$ exit/catch paths and $R$ heap allocations, there are **$E 	imes R$ manual cleanup sites**—adding a single new `throw` path or `catch` handler without copying all `delete` statements immediately causes a memory leak. RAII (`std::unique_ptr`) reduces cleanup sites to **zero**.
