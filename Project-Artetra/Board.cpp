/*
 * Author: Brighton Ancelin
 * 
 * Implementation of the Board data structure.
 */

#include "Board.h"

#include<assert.h>
#include<bitset>

uint8_t Board::_CHAIN_TO_PTS[];

std::ostream& operator<<(std::ostream& os, const Board& board) {
    bool active[Board::COLS];
    for(uint8_t col = 0; col < Board::COLS; ++col) {
        active[col] = board.get_pos_bit(Board::ROWS, col);
    }
    for(uint8_t row_inv = 0; row_inv < Board::ROWS; ++row_inv) {
    	uint8_t row = Board::ROWS - row_inv - 1;
        for(uint8_t col = 0; col < Board::COLS; ++col) {
            if(active[col]) {
                if(board.get_pos_bit(row, col)) {
                    os << "T";
                } else {
                    os << "F";
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

void Board::init_constants() {
    /* The matrix must not have more than 64 bits (represented as uint64_t). */
    assert(Board::MATRIX_SIZE_BITS <= 64);
    /* There must be some points for heuristics. */
    assert(Board::WIN_CHAIN >= 3);
    /* WIN_CHAIN of max 6 requires heuristic scores to be represented as
     * uint16_t. Next would be max 10, which requires heuristic scores to be
     * represented as uint32_t.
     */
	assert(Board::WIN_CHAIN <= 6);
    /* A chain of 2 is always given 1 point. */
    Board::_CHAIN_TO_PTS[0] = 1;
    for(uint8_t i = 1; i < (Board::WIN_CHAIN - 2); ++i) {
        Board::_CHAIN_TO_PTS[i] = Board::_CHAIN_TO_PTS[i-1];
        Board::_CHAIN_TO_PTS[i] *= Board::WIN_CHAIN - 1;
    }
}

uint8_t Board::popcnt(uint64_t elem) {
	std::bitset<64> temp(elem);
	return temp.count();
}

const uint8_t Board::CHAIN_TO_PTS(uint8_t chain_len) {
    if(nullptr == Board::_CHAIN_TO_PTS)
        Board::init_constants();
    if(chain_len < 2)
        return 0;
    return Board::_CHAIN_TO_PTS[chain_len - 2];
}

Board Board::blank_board() {
    Board board;
    for(uint8_t col = 0; col < Board::COLS; ++col) {
        board.set_pos_bit(0, col);
    }
    return board;
}

void Board::set_pos_bit(uint8_t row, uint8_t col) {
    uint8_t bit_index = ((Board::ROWS + 1) * col) + row;
    this->matrix |= (1 << bit_index);
}

bool Board::get_pos_bit(uint8_t row, uint8_t col) const {
    uint8_t bit_index = ((Board::ROWS + 1) * col) + row;
    return 0 != (this->matrix & (1 << bit_index));
}

void Board::raise_occ_mask(uint8_t col) {
    uint8_t bottom_bit_index = (Board::ROWS + 1) * col;
    this->occupied_mask |= this->occupied_mask + (1 << bottom_bit_index);
}

bool Board::col_open(uint8_t col) const {
    return !this->get_pos_bit(Board::ROWS, col);
}

bool Board::is_winning_move(uint8_t col) const {
    /* IMPORTANT: Assumes the column is open. If it is not, results will
     * reflect an unchanged board.
     */
    uint8_t bottom_bit_index = (Board::ROWS + 1) * col;
    /* Add the new piece and to the board. The 1 in the above row is neglected.
     */
    uint64_t proposal = this->occupied_mask + (1 << bottom_bit_index);
    proposal &= ((1 << Board::ROWS) - 1) << bottom_bit_index;
    proposal |= (this->matrix & this->occupied_mask);

    /* Variables used in coming checks. */
    uint8_t shift_remnant;
    uint64_t intermediate;

    /* Check vertically. */
    shift_remnant = Board::WIN_CHAIN - 2;
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
    shift_remnant = Board::WIN_CHAIN - 2;
    intermediate = proposal & (proposal << (Board::ROWS + 1));
    for(uint8_t i = 2; shift_remnant > 0; i <<= 1) {
        if(shift_remnant >= i) {
            intermediate = intermediate &
                    (intermediate << (i * (Board::ROWS + 1)));
            shift_remnant -= i;
        } else {
            intermediate = intermediate &
                    (intermediate << (shift_remnant * (Board::ROWS + 1)));
            break;
        }
    }
    if(0 != intermediate)
        return true;

    /* Check diagonally (one way). */
    shift_remnant = Board::WIN_CHAIN - 2;
    intermediate = proposal & (proposal << Board::ROWS);
    for(uint8_t i = 2; shift_remnant > 0; i <<= 1) {
        if(shift_remnant >= i) {
            intermediate = intermediate & (intermediate << (i * Board::ROWS));
            shift_remnant -= i;
        } else {
            intermediate = intermediate &
                    (intermediate << (shift_remnant * Board::ROWS));
            break;
        }
    }
    if(0 != intermediate)
        return true;

    /* Check diagonally (other way). */
    shift_remnant = Board::WIN_CHAIN - 2;
    intermediate = proposal & (proposal << (Board::ROWS + 2));
    for(uint8_t i = 2; shift_remnant > 0; i <<= 1) {
        if(shift_remnant >= i) {
            intermediate = intermediate &
                    (intermediate << (i * (Board::ROWS + 2)));
            shift_remnant -= i;
        } else {
            intermediate = intermediate &
                    (intermediate << (shift_remnant * (Board::ROWS + 2)));
            break;
        }
    }
    if(0 != intermediate)
        return true;

    return false;
}

uint8_t Board::heuristic_pts() const {
	/* Mask of all bottom row positions. */
	uint64_t bottom_mask = 1;
	for(uint8_t col = 1; col < Board::COLS; ++col) {
		bottom_mask <<= Board::ROWS + 1;
		bottom_mask |= 1;
	}
	/* Mask of all next drop locations. */
	uint64_t next_drops_mask = this->occupied_mask + bottom_mask;
	/* Remove any filled column bits. */
	next_drops_mask &= ~(bottom_mask << Board::ROWS);

	uint64_t masked_matrix = this->matrix & this->occupied_mask;

	/* Variables used in coming accumulations. */
	uint16_t heuristic_score = 0;
	uint8_t cur_pt_val;
	uint64_t intermediate_matrix;
	uint64_t intermediate_mask;

	/* Check vertically. */
	cur_pt_val = 1;
	intermediate_matrix = masked_matrix & (masked_matrix << 1);
	intermediate_mask = (next_drops_mask >> 1) | next_drops_mask;
	heuristic_score += Board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
	for(uint8_t i = 3; i < Board::WIN_CHAIN; ++i) {
		cur_pt_val *= Board::WIN_CHAIN - 1;
		intermediate_matrix &= (intermediate_matrix << 1);
		intermediate_mask |= (intermediate_mask << 1);
		heuristic_score += Board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
	}

	/* Check horizontally (positive). */
	cur_pt_val = 1;
	intermediate_matrix = masked_matrix & (masked_matrix << (Board::ROWS + 1));
	intermediate_mask = (next_drops_mask >> (Board::ROWS + 1)) | next_drops_mask;
	heuristic_score += Board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
	for(uint8_t i = 3; i < Board::WIN_CHAIN; ++i) {
		cur_pt_val *= Board::WIN_CHAIN - 1;
		intermediate_matrix &= (intermediate_matrix << (Board::ROWS + 1));
		intermediate_mask |= (intermediate_mask << (Board::ROWS + 1));
		heuristic_score += Board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
	}

	/* Check horizontally (negative). */
	/* Possibly remove this one... Only adds marginal value */
	cur_pt_val = 1;
	intermediate_matrix = masked_matrix & (masked_matrix >> (Board::ROWS + 1));
	intermediate_mask = (next_drops_mask << (Board::ROWS + 1)) | next_drops_mask;
	heuristic_score += Board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
	for(uint8_t i = 3; i < Board::WIN_CHAIN; ++i) {
		cur_pt_val *= Board::WIN_CHAIN - 1;
		intermediate_matrix &= (intermediate_matrix >> (Board::ROWS + 1));
		intermediate_mask |= (intermediate_mask >> (Board::ROWS + 1));
		heuristic_score += Board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
	}

	/* Check diagonally (one way). */
	cur_pt_val = 1;
	intermediate_matrix = masked_matrix & (masked_matrix << Board::ROWS);
	intermediate_mask = (next_drops_mask >> Board::ROWS) | next_drops_mask;
	heuristic_score += Board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
	for(uint8_t i = 3; i < Board::WIN_CHAIN; ++i) {
		cur_pt_val *= Board::WIN_CHAIN - 1;
		intermediate_matrix &= (intermediate_matrix << Board::ROWS);
		intermediate_mask |= (intermediate_mask << Board::ROWS);
		heuristic_score += Board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
	}

	/* Check diagonally (other way). */
	cur_pt_val = 1;
	intermediate_matrix = masked_matrix & (masked_matrix << (Board::ROWS + 2));
	intermediate_mask = (next_drops_mask >> (Board::ROWS + 2)) | next_drops_mask;
	heuristic_score += Board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
	for(uint8_t i = 3; i < Board::WIN_CHAIN; ++i) {
		cur_pt_val *= Board::WIN_CHAIN - 1;
		intermediate_matrix &= (intermediate_matrix << (Board::ROWS + 2));
		intermediate_mask |= (intermediate_mask << (Board::ROWS + 2));
		heuristic_score += Board::popcnt(intermediate_matrix & intermediate_mask) * cur_pt_val;
	}

	return heuristic_score;
}

void Board::drop(uint8_t col) {
    /* IMPORTANT: Assumes the column is open. If it is not, the board will
     * become inaccurate.
     */
    /* Raise the occupied mask by one row at the specified column. */
    this->raise_occ_mask(col);
    /* Raise an additional row for the new column cap 1. */
    uint8_t bottom_bit_index = (Board::ROWS + 1) * col;
    uint64_t temp_mask = this->occupied_mask;
    temp_mask |= this->occupied_mask + (1 << bottom_bit_index);
    /* Flip the board for negamax to work.
     * Thanks to the previous line, also flip the specified column to
     * 0 (the opponent's piece) and the above row to 1 to indicate the end of
     * the column.
     */
    this->matrix ^= temp_mask;
}

void Board::child(uint8_t col, Board& child) const {
    child = Board(*this);
    child.drop(col);
}


