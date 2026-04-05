#include "Solarfox.hpp"

namespace arcade::bonus {

void Solarfox::initializeBonuses() {
    if (!rules_.bonus)
        return;
    for (const Point point : {Point{9, 8}, Point{15, 8}, Point{9, 12}, Point{15, 12}})
        board_.at(point) = Cell::Obstacle;
    const SolarShotType types[]{SolarShotType::Slow, SolarShotType::Fast, SolarShotType::Spread,
                                SolarShotType::Standard};
    for (std::size_t index = 0; index < guns_.size(); ++index)
        guns_[index].type = types[index % 4];
}

void Solarfox::fireGun(const EnemyGun &gun) {
    const std::size_t count = rules_.bonus && gun.type == SolarShotType::Spread ? 3 : 1;
    if (enemyShots_.size() + count > 256)
        return;
    enemyShots_.push_back({gun.position, gun.direction, Width + Height, gun.type, 0});
    if (rules_.bonus && gun.type == SolarShotType::Spread) {
        const Point perpendicular{-gun.direction.y, gun.direction.x};
        enemyShots_.push_back(
            {gun.position, add(gun.direction, perpendicular), Width + Height, gun.type, 0});
        enemyShots_.push_back({gun.position,
                               add(gun.direction, {-perpendicular.x, -perpendicular.y}),
                               Width + Height, gun.type, 0});
    }
}

void Solarfox::moveGuns() {
    for (auto &gun : guns_) {
        board_.at(gun.position) = Cell::Wall;
        const bool vertical = gun.direction.x != 0;
        auto &coordinate = vertical ? gun.position.y : gun.position.x;
        const int minimum = Margin;
        const int maximum = (vertical ? Height : Width) - Margin - 1;
        if (coordinate + gun.patrol < minimum || coordinate + gun.patrol > maximum)
            gun.patrol = -gun.patrol;
        coordinate += gun.patrol;
        board_.at(gun.position) = Cell::EnemyGun;
    }
}

void Solarfox::moveBonusShots(double seconds) {
    for (auto &shot : enemyShots_) {
        const double scale = shot.type == SolarShotType::Slow   ? 1.8
                             : shot.type == SolarShotType::Fast ? 0.55
                                                                : 1.0;
        const double period = rules_.enemyShotPeriod * scale;
        shot.clock += seconds;
        if (shot.clock + 1e-9 >= period) {
            shot.clock -= period;
            advanceShot(shot);
            intercept();
            // Check every intermediate impact; a fast shot must not tunnel through the ship.
            collisions();
            if (status_ != Status::Running)
                return;
        }
    }
}

} // namespace arcade::bonus
