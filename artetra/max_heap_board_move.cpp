/*
 * Author: Brighton Ancelin
 * 
 * Implementation of the max heap of boards plus moves data structure.
 */

#include <assert.h>

#include "max_heap_board_move.h"

max_heap_board_move::max_heap_board_move(uint8_t capacity) {
    this->node_arr = (max_heap_board_move::node*) malloc(capacity * sizeof(max_heap_board_move::node));
    this->size = 0;
}

max_heap_board_move::~max_heap_board_move() {
    free(this->node_arr);
}

void max_heap_board_move::push(const board& base_board, const uint8_t& col) {
    base_board.spawn(col, &(this->node_arr[this->size].state));
    this->node_arr[this->size].heuristic = this->node_arr[this->size].state.heuristic_pts();
    this->node_arr[this->size].move = col;
    this->swim(this->size);
    ++this->size;
}

std::pair<board, uint8_t> max_heap_board_move::pop() {
    --this->size;
    if(0 != this->size) {
        this->swap(0, this->size);
        this->sink(0);
    }
    return std::make_pair(this->node_arr[this->size].state, this->node_arr[this->size].move);
}

void max_heap_board_move::swap(uint8_t i, uint8_t j) {
    std::swap(this->node_arr[i], this->node_arr[j]);
}

void max_heap_board_move::swim(uint8_t i) {
    if(0 == i) {
        return;
    }
    uint8_t parent = (i - 1) / 2;
    if(this->node_arr[parent].heuristic < this->node_arr[i].heuristic) {
        this->swap(parent, i);
        this->swim(parent);
    }
}

void max_heap_board_move::sink(uint8_t i) {
    const uint8_t child_a = (2 * i) + 1;
    if(child_a < this->size) {
        uint8_t child_b = child_a + 1;
        if(child_b < this->size) {
            if(this->node_arr[child_a].heuristic >= this->node_arr[child_b].heuristic) {
                if(this->node_arr[i].heuristic < this->node_arr[child_a].heuristic) {
                    this->swap(i, child_a);
                    this->sink(child_a);
                }
            } else {
                if(this->node_arr[i].heuristic < this->node_arr[child_b].heuristic) {
                    this->swap(i, child_b);
                    this->sink(child_b);
                }
            }
        } else {
            if(this->node_arr[i].heuristic < this->node_arr[child_a].heuristic) {
                this->swap(i, child_a);
                this->sink(child_a);
            }
        }
    }
}
