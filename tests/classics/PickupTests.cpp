#include "Cases.hpp"
#include "games/classics/TimedPickup.hpp"
#include <limits>
#include <stdexcept>

namespace arcade::test {
void pickupTests() {
    bonus::TimedPickup pickup(2, 3);
    pickup.advance(2);
    requireClassic(pickup.due(), "pickup spawn countdown");
    pickup.place({4, 5});
    pickup.advance(2.9);
    requireClassic(pickup.active(), "pickup expires too soon");
    pickup.advance(0.2);
    requireClassic(!pickup.active() && !pickup.due(), "pickup expiry/cooldown");
    pickup.advance(1.9);
    requireClassic(pickup.due(), "pickup expiry overflow was discarded");
    pickup.place({1, 2});
    requireClassic(!pickup.consume({1, 3}) && pickup.consume({1, 2}), "pickup collision");
    requireClassic(!pickup.consume({1, 2}), "pickup credited twice");
    pickup.reset();
    requireClassic(!pickup.active() && !pickup.due(), "pickup reset");
    bool rejected = false;
    try {
        pickup.advance(std::numeric_limits<double>::infinity());
    } catch (const std::invalid_argument &) {
        rejected = true;
    }
    requireClassic(rejected, "pickup accepted nonfinite delta");
}
} // namespace arcade::test
