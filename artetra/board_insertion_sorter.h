#include <cstdint>

#include "board.h"

class board_insertion_sorter {
public:
    board_insertion_sorter();
    void add(const board& b);
    const board& get(uint8_t i) const;
    uint8_t size() const;
private:
    uint8_t _size;
    struct {board b; uint8_t heur;} moves[board::COLS];
};
