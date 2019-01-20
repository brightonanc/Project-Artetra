#include <cstdint>

#include "board.h"

class move_insertion_sorter {
public:
    move_insertion_sorter();
    void add(const board& b, uint8_t col);
    const board& get(uint8_t i) const;
    uint8_t get_move_col(uint8_t i) const;
    uint8_t size() const;
private:
    uint8_t _size;
    struct {board b; uint8_t col; uint8_t heur;} moves[board::COLS];
};
