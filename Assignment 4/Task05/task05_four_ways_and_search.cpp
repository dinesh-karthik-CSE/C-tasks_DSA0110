#include <iomanip>
#include <iostream>
#include <string>

class Cart {
    std::string customerName;
    std::string items[10];
    double prices[10];
    int n = 0;
    double discount = 0.0;

public:
    explicit Cart(std::string customerName = "Guest") {
        this->customerName = std::move(customerName); // Disambiguating member vs parameter
    }

    Cart& add(const std::string& item, double price) {
        if (n < 10) {
            items[n] = item;
            prices[n] = price;
            ++n;
        }
        return *this; // Returning self-reference for method chaining
    }

    Cart& setDiscount(double discount) {
        this->discount = discount;
        return *this;
    }

    const std::string& getCustomer() const { return customerName; }

    double total() const {
        double t = 0.0;
        for (int i = 0; i < n; ++i) t += prices[i];
        return t * (100.0 - discount) / 100.0;
    }
};

int main() {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "=== TASK 5.1: FOUR WAYS TO REACH AN OBJECT ===\n";

    // (a) Directly by object name (stack-allocated)
    Cart c1("Ananya");
    c1.add("Apples", 150.0).add("Milk", 60.0).setDiscount(5.0);
    std::cout << "(a) Direct by name (" << c1.getCustomer() << "): Rs. " << c1.total() << "\n";

    // (b) Through a pointer to existing object using ->
    Cart* ptrToC1 = &c1;
    ptrToC1->add("Bread", 45.0);
    std::cout << "(b) Pointer with -> (" << ptrToC1->getCustomer() << "): Rs. " << ptrToC1->total() << "\n";

    // (c) Through an alias reference
    Cart& refToC1 = c1;
    refToC1.add("Eggs", 85.0);
    std::cout << "(c) Reference alias (" << refToC1.getCustomer() << "): Rs. " << refToC1.total() << "\n";

    // (d) Dynamically created with new and released with delete
    Cart* heapCart = new Cart("Vikram");
    heapCart->add("Almonds", 450.0).add("Honey", 250.0).setDiscount(10.0);
    std::cout << "(d) Dynamic new/delete (" << heapCart->getCustomer() << "): Rs. " << heapCart->total() << "\n";

    std::cout << "\n=== TASK 5.2: ARRAY OF POINTERS TO ACTIVE CARTS ===\n";
    Cart c2("Rohan");
    c2.add("Rice 5kg", 380.0).add("Ghee", 520.0).setDiscount(5.0);

    Cart* activeCarts[3] = { &c1, &c2, heapCart };
    const int count = 3;

    // Pointer arithmetic traversal to find cart with highest total
    Cart** maxPtr = activeCarts;
    for (Cart** it = activeCarts + 1; it < activeCarts + count; ++it) {
        if ((*it)->total() > (*maxPtr)->total()) {
            maxPtr = it;
        }
    }

    std::cout << "Highest-value active cart belongs to: " << (*maxPtr)->getCustomer()
              << " with Total = Rs. " << (*maxPtr)->total() << "\n";

    delete heapCart; // Clean up dynamic memory
    return 0;
}
