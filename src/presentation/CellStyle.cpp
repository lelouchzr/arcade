#include "arcade/CellStyle.hpp"

namespace arcade {

CellStyle cellStyle(Cell cell) {
    constexpr Color empty{22, 32, 43}, ink{234, 239, 225}, green{190, 244, 109};
    constexpr Color orange{255, 164, 96}, blue{111, 195, 228}, red{255, 101, 107};
    if (cell >= Cell::One && cell <= Cell::Eight)
        return {empty, blue,
                static_cast<char>('0' + static_cast<int>(cell) - static_cast<int>(Cell::Zero))};
    switch (cell) {
    case Cell::Wall:
        return {{55, 75, 88}, ink, 0};
    case Cell::Head:
        return {green, empty, '@'};
    case Cell::Body:
        return {{87, 159, 111}, ink, 0};
    case Cell::Food:
        return {orange, empty, '*'};
    case Cell::Hidden:
        return {{54, 74, 90}, ink, 0};
    case Cell::Flag:
        return {orange, empty, 'F'};
    case Cell::Mine:
        return {red, empty, '*'};
    case Cell::Player:
        return {green, empty, '@'};
    case Cell::Pellet:
        return {empty, orange, '.'};
    case Cell::PowerPellet:
        return {orange, empty, 'O'};
    case Cell::GhostRed:
        return {red, empty, 'G'};
    case Cell::GhostPink:
        return {{246, 155, 212}, empty, 'G'};
    case Cell::GhostCyan:
        return {blue, empty, 'G'};
    case Cell::GhostOrange:
        return {orange, empty, 'G'};
    case Cell::FrightenedGhost:
        return {{65, 99, 195}, ink, 'G'};
    case Cell::GhostEyes:
        return {empty, ink, ':'};
    case Cell::Home:
        return {{30, 42, 62}, blue, '-'};
    case Cell::Trail:
        return {blue, empty, 0};
    case Cell::Claimed:
        return {{29, 48, 40}, green, 0};
    case Cell::Boundary:
        return {{52, 91, 63}, green, 0};
    case Cell::Qix:
        return {{202, 96, 222}, ink, 'X'};
    case Cell::Spark:
        return {orange, empty, '*'};
    case Cell::Fire:
        return {red, ink, '!'};
    case Cell::Projectile:
        return {empty, ink, '|'};
    case Cell::EnemyProjectile:
        return {empty, red, '|'};
    case Cell::CentipedeHead:
        return {green, empty, '@'};
    case Cell::CentipedeBody:
        return {{83, 146, 95}, ink, 'o'};
    case Cell::Obstacle:
        return {{99, 73, 48}, orange, '#'};
    case Cell::Ship:
        return {blue, empty, '^'};
    case Cell::EnemyGun:
        return {red, empty, 'G'};
    case Cell::Powerup:
        return {orange, empty, '*'};
    case Cell::BadPowerup:
        return {red, empty, '!'};
    case Cell::BonusFood:
        return {{217, 116, 235}, ink, '*'};
    case Cell::Question:
        return {{54, 74, 90}, orange, '?'};
    case Cell::JumpingPlayer:
        return {green, empty, '@'};
    case Cell::JumpingGhost:
        return {blue, ink, 'G'};
    case Cell::SecondPlayer:
        return {{246, 155, 212}, empty, '@'};
    case Cell::SlowProjectile:
        return {empty, orange, 'O'};
    case Cell::FastProjectile:
        return {empty, red, '!'};
    default:
        return {empty, ink, 0};
    }
}

} // namespace arcade
