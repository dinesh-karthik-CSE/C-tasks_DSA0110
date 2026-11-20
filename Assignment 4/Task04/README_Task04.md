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
  1. `p->rate()` $ightarrow$ **`3.5` (Compile-Time / Static Binding):** `Account::rate()` is non-virtual, so the compiler binds `p->rate()` directly to `Account::rate()` using `p`'s static type (`Account*`).
  2. `p->vrate()` $ightarrow$ **`4` (Run-Time / Dynamic Dispatch):** `Account::vrate()` is `virtual`, and `p` points to a `SeniorAccount` object whose `vptr` points to `SeniorAccount`'s vtable, dispatching `SeniorAccount::vrate()` at runtime.
  3. `copy.vrate()` $ightarrow$ **`3.5` (Object Slicing):** `Account copy = s;` constructs a standalone `Account` object by copying only the `Account` slice of `s`. `copy`'s `vptr` points to `Account`'s vtable, so `copy.vrate()` calls `Account::vrate()`.

---

## Alternative Concept and Complexity Analysis

### Alternative Approach: `std::vector<std::unique_ptr<Account>>` vs Raw `Account*` Array
* **Time Complexity:** Processing $n$ accounts takes $\mathcal{O}(n)$ time in both designs ($\mathcal{O}(1)$ vtable lookup per element). `std::unique_ptr` is a zero-overhead abstraction in optimized builds (`-O2`), compiling to identical machine instructions as raw `Account*`.
* **Space Complexity:** $\mathcal{O}(n)$ pointers (`sizeof(std::unique_ptr<Account>) == sizeof(Account*) == 8 bytes` on 64-bit systems), plus heap storage for each account.
* **Which of the Three Defects Can Still Occur?**
  * **Defect 1 (Non-virtual `rate()`)** and **Defect 3 (Object Slicing via `Account copy = *accounts[1];`)** can **still occur** if `virtual` is omitted or if an object is dereferenced and copied by value.
  * **Defect 2 (Forgetting `delete` / exception-unsafe leaks)** is eliminated by RAII, *provided* `~Account()` is still `virtual` (since `std::default_delete<Account>` executes `delete ptr;`, which still requires a virtual destructor on `Account`).
