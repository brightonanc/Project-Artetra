/*
 * Author: Brighton Ancelin
 * 
 * Implementation of the board data structure.
 */

#include <assert.h>
#include <bitset>

#include "board.h"

uint8_t board::_CHAIN_TO_PTS[];

std::ostream& operator<<(std::ostream& os, const board& board) {
    bool active[board::COLS];
    for(uint8_t col = 0; col < board::COLS; ++col) {
        active[col] = board.get_pos_bit(board::ROWS, col);
    }
    for(uint8_t row_inv = 0; row_inv < board::ROWS; ++row_inv) {
        uint8_t row = board::ROWS - row_inv - 1;
        for(uint8_t col = 0; col < board::COLS; ++col) {
            if(active[col]) {
                if(board.get_pos_bit(row, col)) {
                    os << "X";
                } else {
                    os << "O";
                }
            } else {
                os << " ";
            }
            if(board.get_pos_bit(row, col)) {
                active[col] = true;
            }
        }
        os << "\n";
    }
    return os;
}

void board::init_constants() {
    /* The matrix must not have more than 64 bits (represented as uint64_t). */
    assert(board::MATRIX_SIZE_BITS <= 64);
    /* There must be some points for heuristics. */
    assert(board::WIN_CHAIN >= 3);
    /* WIN_CHAIN of max 6 requires heuristic scores to be represented as
     * uint16_t. Next would be max 10, which requires heuristic scores to be
     * represented as uint32_t.
     */
    assert(board::WIN_CHAIN <= 6);
    /* A chain of 2 is always given 1 point. */
    board::_CHAIN_TO_PTS[0] = 1;
    for(uint8_t i = 1; i < (board::WIN_CHAIN - 2); ++i) {
        board::_CHAIN_TO_PTS[i] = board::_CHAIN_TO_PTS[i-1];
        board::_CHAIN_TO_PTS[i] *= board::WIN_CHAIN - 1;
    }
}

uint8_t board::popcnt(uint64_t elem) {
    uint8_t count;
    for(count = 0; 0 != elem; ++count) {
        elem &= elem - 1;
    };
    return count;
//    std::bitset<64> temp(elem);
//    return temp.count();
}

const uint8_t board::CHAIN_TO_PTS(uint8_t chain_len) {
    if(nullptr == board::_CHAIN_TO_PTS)
        board::init_constants();
    if(chain_len < 2)
        return 0;
    return board::_CHAIN_TO_PTS[chain_len - 2];
}

board board::blank_board() {
    board b;
    for(uint8_t col = 0; col < board::COLS; ++col) {
        b.set_pos_bit(0, col);
    }
    return b;
}

void board::set_pos_bit(uint8_t row, uint8_t col) {
    uint8_t bit_index = ((board::ROWS + 1) * col) + row;
    this->matrix |= (board::U64_ONE << bit_index);
}

bool board::get_pos_bit(uint8_t row, uint8_t col) const {
    uint8_t bit_index = ((board::ROWS + 1) * col) + row;
    return 0 != (this->matrix & (board::U64_ONE << bit_index));
}

void board::raise_occ_mask(uint8_t col) {
    uint8_t bottom_bit_index = (board::ROWS + 1) * col;
    this->occupied_mask |= this->occupied_mask + (board::U64_ONE << bottom_bit_index);
}

bool board::col_open(uint8_t col) const {
    return !this->get_pos_bit(board::ROWS, col);
}

bool board::is_winning_move(uint8_t col) const {
    /* IMPORTANT: Assumes the column is open. If it is not, results will
     * reflect an unchanged board.
     */
    uint8_t bottom_bit_index = (board::ROWS + 1) * col;
    /* Add the new piece and to the board. The 1 in the above row is neglected.
     */
    uint64_t proposal = this->occupied_mask + (board::U64_ONE << bottom_bit_index);
    proposal &= ((board::U64_ONE << board::ROWS) - board::U64_ONE) << bottom_bit_index;
    proposal |= (this->matrix & this->occupied_mask);

    /* Variables used in coming checks. */
    uint8_t shift_remnant;
    uint64_t intermediate;

    /* Check vertically. */
    shift_remnant = board::WIN_CHAIN - 2;
    intermediate = proposal & (proposal << 1);
    for(uint8_t i = 2; shift_remnant > 0; i <<= 1) {
        if(shift_remnant >= i) {
            intermediate = intermediate & (intermediate << i);
            shift_remnant -= i;
        } else {
            intermediate = intermediate & (intermediate << shift_remnant);
            break;
        }
    }
    if(0 != intermediate)
        return true;

    /* Check horizontally. */
    shift_remnant = board::WIN_CHAIN - 2;
    intermediate = proposal & (proposal << (board::ROWS + 1));
    for(uint8_t i = 2; shift_remnant > 0; i <<= 1) {
        if(shift_remnant >= i) {
            intermediate = intermediate &
                    (intermediate << (i * (board::ROWS + 1)));
            shift_remnant -= i;
        } else {
            intermediate = intermediate &
                    (intermediate << (shift_remnant * (board::ROWS + 1)));
            break;
        }
    }
    if(0 != intermediate)
        return true;

    /* Check diagonally (one way). */
    shift_remnant = board::WIN_CHAIN - 2;
    intermediate = proposal & (proposal << board::ROWS);
    for(uint8_t i = 2; shift_remnant > 0; i <<= 1) {
        if(shift_remnant >= i) {
            intermediate = intermediate & (intermediate << (i * board::ROWS));
            shift_remnant -= i;
        } else {
            intermediate = intermediate &
                    (intermediate << (shift_remnant * board::ROWS));
            break;
        }
    }
    if(0 != intermediate)
        return true;

    /* Check diagonally (other way). */
    shift_remnant = board::WIN_CHAIN - 2;
    intermediate = proposal & (proposal << (board::ROWS + 2));
    for(uint8_t i = 2; shift_remnant > 0; i <<= 1) {
        if(shift_remnant >= i) {
            intermediate = intermediate &
                    (intermediate << (i * (board::ROWS + 2)));
            shift_remnant -= i;
        } else {
            intermediate = intermediate &
                    (intermediate << (shift_remnant * (board::ROWS + 2)));
            break;
        }
    }
    if(0 != intermediate)
        return true;

    return false;
}

uint8_t board::heuristic_pts() const {
//    return 0;
    // Mask of all bottom row positions.
    uint64_t bottom_mask = 1;
    for(uint8_t col = 1; col < board::COLS; ++col) {
        bottom_mask <<= board::ROWS + 1;
        bottom_mask |= 1;
    }
    // Mask of all next drop locations.
    uint64_t next_drops_mask = this->occupied_mask + bottom_mask;
    // Remove any filled column bits.
    next_drops_mask &= ~(bottom_mask << board::ROWS);

    uint64_t masked_matrix = this->matrix & this->occupied_mask;

    // Variables used in coming accumulations.
    uint16_t heuristic_score = 0;
    uint8_t cur_pt_val;
    uint64_t intermediate_matrix;
    uint64_t intermediate_mask;

    // Check vertically.
    cur_pt_val = 1;
    intermediate_matrix = masked_matrix & (masked_matrix << 1);
    intermediate_mask = (next_drops_mask >> 1) | next_drops_mask;
    heuristic_score += board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
    for(uint8_t i = 3; i < board::WIN_CHAIN; ++i) {
        cur_pt_val *= board::WIN_CHAIN - 1;
        intermediate_matrix &= (intermediate_matrix << 1);
        intermediate_mask |= (intermediate_mask << 1);
        heuristic_score += board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
    }

    // Check horizontally (positive).
    cur_pt_val = 1;
    intermediate_matrix = masked_matrix & (masked_matrix << (board::ROWS + 1));
    intermediate_mask = (next_drops_mask >> (board::ROWS + 1)) | next_drops_mask;
    heuristic_score += board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
    for(uint8_t i = 3; i < board::WIN_CHAIN; ++i) {
        cur_pt_val *= board::WIN_CHAIN - 1;
        intermediate_matrix &= (intermediate_matrix << (board::ROWS + 1));
        intermediate_mask |= (intermediate_mask << (board::ROWS + 1));
        heuristic_score += board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
    }

    // Check horizontally (negative).
    // Possibly remove this one... Only adds marginal value
    cur_pt_val = 1;
    intermediate_matrix = masked_matrix & (masked_matrix >> (board::ROWS + 1));
    intermediate_mask = (next_drops_mask << (board::ROWS + 1)) | next_drops_mask;
    heuristic_score += board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
    for(uint8_t i = 3; i < board::WIN_CHAIN; ++i) {
        cur_pt_val *= board::WIN_CHAIN - 1;
        intermediate_matrix &= (intermediate_matrix >> (board::ROWS + 1));
        intermediate_mask |= (intermediate_mask >> (board::ROWS + 1));
        heuristic_score += board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
    }

    // Check diagonally (one way).
    cur_pt_val = 1;
    intermediate_matrix = masked_matrix & (masked_matrix << board::ROWS);
    intermediate_mask = (next_drops_mask >> board::ROWS) | next_drops_mask;
    heuristic_score += board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
    for(uint8_t i = 3; i < board::WIN_CHAIN; ++i) {
        cur_pt_val *= board::WIN_CHAIN - 1;
        intermediate_matrix &= (intermediate_matrix << board::ROWS);
        intermediate_mask |= (intermediate_mask << board::ROWS);
        heuristic_score += board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
    }

    // Check diagonally (other way).
    cur_pt_val = 1;
    intermediate_matrix = masked_matrix & (masked_matrix << (board::ROWS + 2));
    intermediate_mask = (next_drops_mask >> (board::ROWS + 2)) | next_drops_mask;
    heuristic_score += board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
    for(uint8_t i = 3; i < board::WIN_CHAIN; ++i) {
        cur_pt_val *= board::WIN_CHAIN - 1;
        intermediate_matrix &= (intermediate_matrix << (board::ROWS + 2));
        intermediate_mask |= (intermediate_mask << (board::ROWS + 2));
        heuristic_score += board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
    }

    return heuristic_score;
}

void board::drop(uint8_t col) {
    /* IMPORTANT: Assumes the column is open. If it is not, the board will
     * become inaccurate.
     */
    /* Raise the occupied mask by one row at the specified column. */
    this->raise_occ_mask(col);
    /* Raise an additional row for the new column cap 1. */
    uint8_t bottom_bit_index = (board::ROWS + 1) * col;
    uint64_t temp_mask = this->occupied_mask;
    temp_mask |= this->occupied_mask + (board::U64_ONE << bottom_bit_index);
    /* Flip the board for negamax to work.
     * Thanks to the previous line, also flip the specified column to
     * 0 (the opponent's piece) and the above row to 1 to indicate the end of
     * the column.
     */
    this->matrix ^= temp_mask;
}

void board::spawn(uint8_t col, board* const child) const {
    (*child) = board(*this);
    child->drop(col);
}

board board::inverse() const {
    board inverted(*this);
    inverted.matrix ^= inverted.occupied_mask;
    return inverted;
}


