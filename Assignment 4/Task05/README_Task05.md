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
