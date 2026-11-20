# Task B4 – Final Engineering Decision (5 Marks)

## B4.1 & B4.2 – Practical Extension & AddressSanitizer Verification

* **Added Without Modifying Any Existing Class:**
  1. `TeleMedicineBill : public Bill` (charges per consultation minute + e-prescription fee).
  2. `CorporateTieUp : public Payer` (covers employer subsidy percentage up to an annual corporate cap).
* **AddressSanitizer (`-fsanitize=address`) Report:** Executed with both valid settlements and thrown/caught exceptions (`BillingValidationError` and `ClaimError`). Program exited with code `0` and **zero memory leaks or type mismatches**.

---

## B4.3 – Production Decision for the 15-Hospital Group (200,000 Bills/Day)

**Verdict: YES — Runtime Polymorphism (`virtual` Base Classes + RAII) with Exceptions is the optimal architecture for this hospital group.**

### Quantitative & Architectural Justification
1. **Throughput Capacity (200,000 Bills/Day):** Based on our Task B3 benchmarks (`23.465 ms` for 1,000,000 bills under `-O2`), processing all **200,000 daily bills** across 15 hospitals takes **~4.7 milliseconds total CPU time** (~23.5 ns per bill)—less than $0.00001\%$ of a single CPU core's daily capacity.
2. **Zero-Regression Annual Extensibility (2 New Bills + 3 New Payers / Year):** Under the Open/Closed Principle, adding 5 new classes every year requires **zero modifications** to tested billing engines, existing bill classes, or settlement loops, eliminating regression bugs across 15 hospitals.
3. **Fault Isolation & Resource Safety:** Because genuine billing/claim errors occur in a small fraction (< 1–2%) of transactions, C++ exceptions incur negligible happy-path overhead while guaranteeing via RAII (`std::unique_ptr`, `std::vector`) that a single rejected claim never leaks memory or aborts batch processing.

---

## Alternative Concept and Complexity Analysis (Data-Driven Rule Engine / Config Plug-ins vs C++ Classes)

| Dimension | Compiled C++ Subclasses (`Bill` / `Payer`) | Configuration Plug-in Rule Engine (JSON/DSL Interpreter) |
| :--- | :--- | :--- |
| **Time per Bill** | $\mathcal{O}(1)$ — native machine instructions via single vtable jump (~23 ns). | $\mathcal{O}(R)$ — AST/expression evaluation, hash-map variable lookups, and dynamic dispatch per formula rule (~500–5,000 ns, **20$	imes$–200$	imes$ slower**). |
| **Space for Rules** | Zero runtime rule-table overhead (logic compiled into read-only `.text` segment). | $\mathcal{O}(T \cdot R)$ heap memory to store parsed rule trees, variable bindings, and metadata for $T$ bill types. |
| **Effort & Risk of Extension** | Requires C++ coding, unit tests, and binary redeployment, **but** provides **compile-time static type checking** (`override`, strong types) preventing malformed billing formulas in production. | Zero recompilation (hospital admins edit config files), **but** carries **high runtime risk** (syntax errors, division by zero, or silent miscalculations in untested config files). **Hybrid Best Practice:** Keep structural bill types as C++ subclasses while loading numeric tariff rates/caps from validated config files. |
