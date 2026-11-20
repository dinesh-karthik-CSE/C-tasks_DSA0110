# Task B1 – Implement and Validate Polymorphic Billing (7 Marks)

## B1.1 – Hand-Verified Settlement Matrix (4 Bill Types $	imes$ 3 Payers)

| Bill ID | Category | Formula & Hand Calculation | Gross Bill (Rs.) | Self-Pay Covered | Insurance Covered (30k cap, 10% copay) | Govt Scheme Covered |
| :--- | :--- | :--- | ---: | ---: | ---: | ---: |
| **B101** | Out-Patient | $600 + 1400$ | **2000.00** | 0.00 | $2000 	imes 0.90 = \mathbf{1800.00}$ | $\min(2000, 1000) = \mathbf{1000.00}$ |
| **B102** | In-Patient | $(4 	imes 3000) + 25000 + 5000$ | **42000.00** | 0.00 | $\min(42000, 30000) 	imes 0.90 = \mathbf{27000.00}$ | $\min(42000, 35000) = \mathbf{35000.00}$ |
| **B103** | Day-Care | Package `CATARACT` | **22000.00** | 0.00 | $22000 	imes 0.90 = \mathbf{19800.00}$ | $\min(22000, 18000) = \mathbf{18000.00}$ |
| **B104** | Emergency | $4500 + (5 	imes 1500)$ | **12000.00** | 0.00 | $12000 	imes 0.90 = \mathbf{10800.00}$ | $\min(12000, 12000) = \mathbf{12000.00}$ |

All 12 program outputs match hand calculations to the exact paisa.

---

## B1.2 – Comparison with Part A Design
* **Refinement Made:** In Part A (`A3.1`), `Insurance::amountCovered()` threw `ClaimError` whenever `bill.total() > coverLimit`. During implementation of the full settlement matrix (`B1.1`), capping eligible coverage at `std::min(bill.total(), coverLimit)` allows partial insurance settlement (`Rs. 27,000.00` on a `Rs. 42,000.00` bill, leaving `Rs. 15,000.00` patient co-pay) for routine billing, while reserving `ClaimLimitExceededError` for strict cashless pre-authorization checks (tested in `Task B2`).

---

## B1.3 & Complete the Code – Removing `virtual` from `total()`

1. **In `Complete the Code` (`double total() const = 0;` $ightarrow$ `double total() const;` without `= 0` or `virtual`):**
   * If `double total() const = 0;` has `virtual` removed while keeping `= 0`, `g++` emits a compile error:
     `error: initializer specified for non-virtual method 'double Bill::total() const'`.
   * If `virtual` is removed and a dummy `double total() const { return 0; }` is provided in `Bill`, `b->print()` and `b->total()` statically bind to `Bill::total()`, printing **`OP Ravi: 0`**, **`IP Meena: 0`**, and **`Total 0`** instead of `49200`.

---

## B1.4 – Removing `virtual` from `~Bill()` under `-fsanitize=address`
* Without `virtual ~Bill()`, `delete b;` invokes `~Bill()` only, skipping `~InPatientLeak()` and leaking `new double[50]` (`400 bytes`).
* AddressSanitizer immediately aborts with:
  `ERROR: AddressSanitizer: new-delete-type-mismatch` (allocated 16 bytes for `InPatientLeak`, deallocated 8 bytes as `Bill`).

---

## Alternative Concept and Complexity Analysis (Templates vs Virtual Functions)

* **Template Approach (`template <typename BillContainer> double sumBills(const BillContainer& c)`):**
  * **Time per Bill:** $\mathcal{O}(1)$ with **zero indirect call overhead**—the compiler monomorphizes separate loops for `std::vector<OutPatientBill>` and `std::vector<InPatientBill>`, enabling full function inlining and SIMD vectorization.
  * **Space Complexity:** No 8-byte `vptr` per bill object, **but** requires **4 separate containers** (`opBills`, `ipBills`, `dcBills`, `emBills`), duplicating loop code in the binary (code bloat).
  * **What Is Lost:** You **cannot store all bills of the day in a single chronological list** (`std::vector<std::unique_ptr<Bill>>`) or process heterogeneous user inputs chosen at runtime without resorting to `std::variant` or `virtual` dispatch.
