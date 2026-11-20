# Task 06 – Design Challenge: Vehicle Rental Fleet

## Stage 1 – Interpret the Problem

### Entities, Attributes & Polymorphic Behaviour
* **Common (`Vehicle`):** `regNo`, `dailyRate`, `rented`, `bookedDays`. Behaviour: `rentOut()`, `returnVehicle()`, `computeCharge()` (varies per vehicle), `printDetails()`.
* **Non-Motorized (`Bicycle`):** `hasGear`. Charge = `days * (dailyRate + gearFee)`.
* **Motorized (`MotorVehicle`):** `energyLevelPercent`, `serviceDueDate` (`Date` member object).
* **Petrol Vehicles (`TwoWheeler`, `Car`):** `TwoWheeler` charges per litre of petrol; `Car` adds `seatingCapacity` and charges per litre + multi-seater insurance.
* **Electric Vehicles (`ElectricCar`, `ElectricBus`):** `batteryCapacityKWh`, `seatingCapacity`/`passengerCapacity`; charged **per kWh** instead of per litre.

### Testable Requirements
1. **REQ-1:** Renting an available vehicle for $d > 0$ days transitions its state to `RENTED`; renting an already-rented vehicle fails gracefully (`returns false`).
2. **REQ-2:** Returning a vehicle after $a$ actual days (when booked for $b$ days) computes `computeCharge(a, energyUsed)` plus a $50\% 	imes 	ext{dailyRate}$ late surcharge for $\max(0, a - b)$ overdue days, and resets state to `AVAILABLE`.
3. **REQ-3:** Adding a new vehicle type (`ElectricBus`) requires zero modifications to `Vehicle`, `MotorVehicle`, `Bicycle`, `TwoWheeler`, `Car`, `ElectricCar`, or `RentalFleet`.

---

## Stage 2 – Proposed Class Hierarchy (UML)

```text
                    +------------------------------------+
                    |           <<abstract>>             |
                    |              Vehicle               |
                    +------------------------------------+
                    | # regNo : string                   |
                    | # dailyRate : double               |
                    | # rented : bool                    |
                    | # bookedDays : int                 |
                    +------------------------------------+
                    | + virtual ~Vehicle()               |
                    | + rentOut(days) : bool             |
                    | + returnVehicle(days, energy)      |
                    | + virtual category() : string = 0  |
                    | + virtual computeCharge(...) = 0   |
                    +------------------------------------+
                              ^                ^
                              |                |
               +--------------+                +------------------+
               | (public)                                         | (public)
+-----------------------------+                    +------------------------------+
|           Bicycle           |                    |         MotorVehicle         |<>---- [Date]
+-----------------------------+                    +------------------------------+   (Has-a composition)
| - hasGear : bool            |                    | # energyLevelPercent : double|
+-----------------------------+                    | # serviceDueDate : Date      |
| + computeCharge(...)        |                    +------------------------------+
+-----------------------------+                       ^          ^          ^
                                                      |          |          |
                                         +------------+          |          +-------------+
                                         |                       |                        |
                              +--------------------+  +--------------------+  +--------------------+
                              |     TwoWheeler     |  |        Car         |  |    ElectricCar     |
                              +--------------------+  +--------------------+  +--------------------+
                              | + computeCharge()  |  | # seatingCapacity  |  | - batteryCapacity  |
                              +--------------------+  | + computeCharge()  |  | + computeCharge()  |
                                                      +--------------------+  +--------------------+
```

---

## Stage 3 – Critical Design Decisions

1. **Abstract Base Class (`Vehicle`):** Declares `category()` and `computeCharge()` as **pure virtual (`= 0`)** so no unspecialized `Vehicle` can be instantiated, while hoisting shared state (`regNo`, `dailyRate`, `rented`) and the Template Method `returnVehicle()` into `Vehicle` to avoid duplicate late-fee logic.
2. **Intermediate `MotorVehicle` Class + `Date` Composition:** Prevents duplicating `energyLevelPercent` and `serviceDueDate` across `TwoWheeler`, `Car`, `ElectricCar`, and `ElectricBus` without forcing `Bicycle` to carry engine/fuel fields.
3. **Virtual Destructor & Fleet Storage:** `RentalFleet` stores `Vehicle* fleet[MAX_VEHICLES]` (an array of base-class pointers) and deletes them via `virtual ~Vehicle() = default;`, guaranteeing leak-free polymorphic destruction.

---

## Stage 4 – SIMATS Standard Algorithmic Pseudocode (Rent & Return)

```text
ALGORITHM RentVehicle(fleet, count, targetReg, days)
INPUT:  fleet[0..count-1] (array of Vehicle pointers), targetReg (string), days (integer)
OUTPUT: boolean status (TRUE if rented, FALSE otherwise)

BEGIN
    IF days <= 0 THEN
        RETURN FALSE
    END IF

    FOR i <- 0 TO count - 1 DO
        IF fleet[i]->getRegNo() = targetReg THEN
            IF fleet[i]->isRented() = TRUE THEN
                RETURN FALSE
            END IF
            fleet[i]->rented <- TRUE
            fleet[i]->bookedDays <- days
            RETURN TRUE
        END IF
    END FOR

    RETURN FALSE
END
```

```text
ALGORITHM ReturnVehicle(vehiclePtr, actualDays, energyUsed)
INPUT:  vehiclePtr (Vehicle pointer), actualDays (integer), energyUsed (real)
OUTPUT: totalBill (real; -1.0 if vehicle was not rented)

BEGIN
    IF vehiclePtr = NULL OR vehiclePtr->isRented() = FALSE THEN
        RETURN -1.0
    END IF

    baseCharge <- vehiclePtr->computeCharge(actualDays, energyUsed)   // Dynamic dispatch

    IF actualDays > vehiclePtr->bookedDays THEN
        extraDays <- actualDays - vehiclePtr->bookedDays
    ELSE
        extraDays <- 0
    END IF

    latePenalty <- extraDays * (vehiclePtr->dailyRate * 0.50)
    totalBill   <- baseCharge + latePenalty

    vehiclePtr->rented     <- FALSE
    vehiclePtr->bookedDays <- 0

    RETURN totalBill
END
```

---

## Alternative Concept and Complexity Analysis

### Alternative Approach: Single Monolithic `Vehicle` Class with `enum VehicleType` and `switch` in `computeCharge()`

| Metric | Abstract Base Class + Virtual Functions | Single `Vehicle` Class + `enum` + `switch` |
| :--- | :--- | :--- |
| **Time per `computeCharge()` Call** | $\mathcal{O}(1)$ — single indirect vtable lookup (`vptr[slot]`). | $\mathcal{O}(1)$ — `switch` jump-table or branch chain. |
| **Space per Object** | Compact: `Bicycle` stores only `Vehicle` fields + `bool hasGear` + 1 `vptr` (8 B); `Bicycle` carries **zero** battery/fuel/seat fields. | Bloated: Every `Bicycle` object must allocate unused memory for `energyLevelPercent`, `serviceDueDate`, `seatingCapacity`, and `batteryCapacityKWh` (+24–32 wasted bytes per object). |
| **Classes Changed When Adding `ElectricBus`** | **0 existing classes modified** (only 1 new subclass `ElectricBus` added — strictly Open/Closed). | **Modifies `Vehicle` header & implementation** (`enum VehicleType`, constructors, new fields, `switch` inside `computeCharge()` and `printDetails()`), forcing full recompilation and retesting of all vehicle types. |
