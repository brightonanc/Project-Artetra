#include "board_insertion_sorter.h"

board_insertion_sorter::board_insertion_sorter() :
        _size(0) {};

void board_insertion_sorter::add(const board& b) {
    uint8_t heur = b.heuristic_pts();
    uint8_t i;
    for(i = this->_size; (i > 0) && (this->moves[i - 1].heur < heur); --i) {
        this->moves[i] = this->moves[i - 1];
    }
    this->moves[i].b = b;
    this->moves[i].heur = heur;
    ++this->_size;
};

/* WARNING: Does not bounds check! */
const board& board_insertion_sorter::get(uint8_t i) const {
    return this->moves[i].b;
};

uint8_t board_insertion_sorter::size() const {
    return this->_size;
};
