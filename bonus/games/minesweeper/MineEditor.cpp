#include "MineEditor.hpp"
#include <algorithm>
#include <array>

namespace arcade::bonus {
void MineEditor::begin(MineConfiguration configuration) {
    configuration_ = configuration;
    selected_ = 0;
    digits_.clear();
    clamp();
}

void MineEditor::clamp() {
    configuration_.width = std::clamp(configuration_.width, 5, 40);
    configuration_.height = std::clamp(configuration_.height, 5, 24);
    configuration_.mines =
        std::clamp(configuration_.mines, 1, configuration_.width * configuration_.height - 9);
    configuration_.seconds = std::clamp(configuration_.seconds, 30, 3600);
}

void MineEditor::assign(int value) {
    std::array<int *, 4> fields{&configuration_.width, &configuration_.height,
                                &configuration_.mines, &configuration_.seconds};
    *fields[static_cast<std::size_t>(selected_)] = value;
    clamp();
}

void MineEditor::adjust(int delta) {
    const std::array<int, 4> values{configuration_.width, configuration_.height,
                                    configuration_.mines, configuration_.seconds};
    assign(values[static_cast<std::size_t>(selected_)] + delta * (selected_ == 3 ? 30 : 1));
    digits_.clear();
}

void MineEditor::handle(const Input &input) {
    if (input.action == Action::Up || input.action == Action::Down) {
        selected_ = (selected_ + (input.action == Action::Up ? 3 : 1)) % 4;
        digits_.clear();
    } else if (input.action == Action::Left || input.action == Action::Right)
        adjust(input.action == Action::Left ? -1 : 1);
    else if (input.action == Action::Text && input.text >= '0' && input.text <= '9') {
        if (digits_.size() == 4)
            digits_.clear();
        digits_.push_back(static_cast<char>(input.text));
        assign(std::stoi(digits_));
    }
}

MineConfiguration MineEditor::configuration() const {
    return configuration_;
}

std::vector<std::string> MineEditor::information() const {
    const std::array<std::string, 4> names{"WIDTH  ", "HEIGHT ", "MINES  ", "TIME   "};
    const std::array<int, 4> values{configuration_.width, configuration_.height,
                                    configuration_.mines, configuration_.seconds};
    std::vector<std::string> result{"CUSTOM DIFFICULTY", "ARROWS EDIT DIGITS TYPE"};
    for (std::size_t i = 0; i < values.size(); ++i)
        result.push_back((static_cast<int>(i) == selected_ ? "> " : "  ") + names[i] +
                         std::to_string(values[i]));
    result.emplace_back("ENTER OK  D/BS CANCEL");
    return result;
}
} // namespace arcade::bonus
