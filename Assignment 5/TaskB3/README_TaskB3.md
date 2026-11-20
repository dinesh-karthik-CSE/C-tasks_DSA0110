# Task B3 – Virtual Functions vs `switch` vs Exceptions: The Cost (8 Marks)

## B3.1 & B3.2 – Measured Dispatch Performance (`-O2` Optimization)

* **10-Bill Correctness Verification:** Both Virtual and `switch` implementations produce the exact same total: **`Rs. 125,820.000`**.

| Bill Count ($N$) | Virtual (`std::vector<unique_ptr<VBill>>`) | Tagged Struct + `switch` (`std::vector<SwitchBill>`) | `std::variant` + `std::visit` (`std::vector<VariantBill>`) |
| ---: | ---: | ---: | ---: |
| **1,000** | 0.006 ms | 0.003 ms | 0.003 ms |
| **1,000,000** | 23.465 ms | 17.590 ms | 16.272 ms |
| **10,000,000** | 291.204 ms | 208.240 ms | 169.469 ms |

### Object Memory Footprint (`sizeof`)
* `sizeof(VOutPatient)` = **24 bytes** (`vptr` 8 B + 2 doubles 16 B)
* `sizeof(VInPatient)` = **32 bytes** (`vptr` 8 B + `int` + padding + 2 doubles)
* `sizeof(SwitchBill)` = **40 bytes** (carries unused fields in every element)
* `sizeof(VariantBill)` = **32 bytes** ($\max(	ext{sizeof}(	ext{VarIP})) = 24	ext{ B} + 	ext{index tag } 1	ext{ B} + 7	ext{ B alignment padding} = 32	ext{ B}$)

---

## B3.3 – Measured Cost of Exceptions vs Return Codes (1,000,000 Bills)

| Invalid Bill Rate | Return Code (`enum ValCode`) | Exceptions (`try` / `throw` / `catch`) | Slowdown Factor |
| :--- | ---: | ---: | ---: |
| **0% invalid (0 throws)** | 6.965 ms | 8.424 ms | $1.21	imes$ (near zero-cost) |
| **1% invalid (10,000 throws)** | 6.935 ms | 31.341 ms | $4.52	imes$ |
| **50% invalid (500,000 throws)** | 5.330 ms | 1159.583 ms | **$217.5	imes$ slower** |

---

## B3.4 – Engineering Trade-Off & Decision Rules

1. **Why `switch` / `std::variant` Beat `virtual` by ~1.4$	imes$–1.7$	imes$ at 10M Elements:** Contiguous `std::vector<SwitchBill>` and `std::vector<VariantBill>` store objects inline in L1/L2 CPU cache with hardware prefetcher locality, whereas `std::vector<std::unique_ptr<VBill>>` chases heap pointers across fragmented memory addresses (cache misses) and prevents compiler inlining across indirect vtable calls.
2. **When to Use Each:**
   * **Use `virtual` Inheritance:** When the set of types is **open for extension** (new bill types/payers added yearly without touching existing code). Even at 10,000,000 bills, 291 ms is **29 nanoseconds per bill**—negligible compared to database/network I/O.
   * **Use `std::variant` / `switch`:** Inside closed, hot inner loops (e.g., physics engines, HFT ticks) where types are fixed at compile time and contiguous cache locality is critical.
   * **Use Exceptions vs Return Codes:** Use **exceptions** for truly exceptional failures (< 1% rate: malformed files, exceeded limits, `bad_alloc`); use **return codes / `std::optional`** when failure is routine (> 5–10% rate, such as high-frequency user input filtering).
