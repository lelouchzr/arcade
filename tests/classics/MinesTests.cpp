#include "Access.hpp"
#include "Cases.hpp"
#include "games/minesweeper/MineEditor.hpp"

namespace arcade::test {
namespace {
void markingAndFacesTests() {
    Minesweeper game;
    game.update(0, {{Action::Secondary, '?'}});
    requireClassic(countClassic(game.snapshot(), Cell::Flag) == 1, "question cycle flag");
    game.update(0, {{Action::Secondary, '?'}});
    requireClassic(countClassic(game.snapshot(), Cell::Question) == 1 &&
                       countClassic(game.snapshot(), Cell::Flag) == 0,
                   "question cycle question");
    game.update(0, {{Action::Secondary, '?'}});
    requireClassic(countClassic(game.snapshot(), Cell::Question) == 0 &&
                       countClassic(game.snapshot(), Cell::Flag) == 0,
                   "question cycle hidden");
    game.update(0, {{Action::Secondary, '?'}, {Action::Secondary, '?'}});
    game.update(0, {{Action::Primary}});
    requireClassic(game.snapshot().score >= 90 &&
                       countClassic(game.snapshot(), Cell::Question) == 0,
                   "question square must reveal normally");
    requireClassic(game.snapshot().information.front().find(":O") != std::string::npos,
                   "reveal face must be scared");
    game.update(0.4, {});
    requireClassic(game.snapshot().information.front().find(":-)") != std::string::npos,
                   "scared face must return to normal");
    for (Status status : {Status::Won, Status::Lost}) {
        MinesBonusAccess::finish(game, status);
        const auto before = game.snapshot();
        game.update(0.2, {{Action::Primary}});
        const auto after = game.snapshot();
        requireHud(after);
        requireClassic(before.score == after.score && before.cells == after.cells &&
                           before.secondsLeft == after.secondsLeft,
                       "end animation must freeze gameplay");
        requireClassic(before.sprites.front().position != after.sprites.front().position,
                       "end animation must advance after terminal status");
        requireClassic(after.information.front().find(status == Status::Won ? "B-)" : "X-(") !=
                           std::string::npos,
                       "end face missing");
    }
}

void customDifficultyTests() {
    Minesweeper game;
    game.update(0, {{Action::Secondary}, {Action::Secondary}});
    requireClassic(countClassic(game.snapshot(), Cell::Flag) == 0 &&
                       countClassic(game.snapshot(), Cell::Question) == 0,
                   "F must remain a simple flag toggle");
    for (int i = 0; i < 3; ++i)
        game.update(0, {{Action::Difficulty}});
    requireClassic(game.snapshot().information.front() == "CUSTOM DIFFICULTY",
                   "custom editor missing");
    game.update(20, {{Action::Text, '7'},
                     {Action::Down},
                     {Action::Text, '5'},
                     {Action::Down},
                     {Action::Text, '9'},
                     {Action::Text, '9'},
                     {Action::Text, '9'},
                     {Action::Down},
                     {Action::Text, '3'},
                     {Action::Text, '0'},
                     {Action::Confirm}});
    requireClassic(game.snapshot().width == 7 && game.snapshot().height == 5 &&
                       game.snapshot().secondsLeft == 30,
                   "custom editor apply dimensions/time");
    game.update(0, {{Action::Primary}});
    const auto custom = game.snapshot();
    requireHud(custom);
    requireClassic(
        custom.cells[static_cast<std::size_t>(custom.cursor.y * custom.width + custom.cursor.x)] ==
            Cell::Zero,
        "dense custom board must preserve first zero");
    requireClassic(custom.information.size() <= 7, "mines HUD must fit all graphics backends");
    Minesweeper cancelled;
    for (int i = 0; i < 3; ++i)
        cancelled.update(0, {{Action::Difficulty}});
    requireClassic(cancelled.snapshot().information.size() <= 7, "custom HUD must fit backends");
    requireHud(cancelled.snapshot());
    cancelled.update(0, {{Action::Right}, {Action::Backspace}});
    requireClassic(cancelled.snapshot().width == 9 &&
                       cancelled.snapshot().information.front().find("FACE") != std::string::npos,
                   "backspace must cancel custom editor without applying edits");
}

void largeBoardTests() {
    Minesweeper large;
    for (int i = 0; i < 3; ++i)
        large.update(0, {{Action::Difficulty}});
    large.update(0, {{Action::Text, '4'},
                     {Action::Text, '0'},
                     {Action::Down},
                     {Action::Text, '2'},
                     {Action::Text, '4'},
                     {Action::Confirm}});
    for (int i = 0; i < 40; ++i)
        large.update(0, {{Action::Right}, {Action::Down}});
    auto camera = large.snapshot().camera;
    requireClassic(camera && !camera->isometric && camera->center == Point{39, 23} &&
                       camera->columns == 30 && camera->rows == 21,
                   "large custom board camera must follow cursor to far edge");
    for (int i = 0; i < 40; ++i)
        large.update(0, {{Action::Left}, {Action::Up}});
    camera = large.snapshot().camera;
    requireClassic(camera && camera->center == Point{0, 0},
                   "large custom board camera must follow cursor back to origin");
}

void editorBoundsTests() {
    bonus::MineEditor editor;
    editor.begin({-10, 1000, 9999, -1});
    const auto clamped = editor.configuration();
    requireClassic(clamped.width == 5 && clamped.height == 24 && clamped.mines == 111 &&
                       clamped.seconds == 30,
                   "custom editor bounds");
}
} // namespace

void minesBonusTests() {
    markingAndFacesTests();
    customDifficultyTests();
    largeBoardTests();
    editorBoundsTests();
}
} // namespace arcade::test
