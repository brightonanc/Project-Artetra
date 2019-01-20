/*
 * Author: Brighton Ancelin
 * 
 * Implementation of the max heap of boards data structure.
 */

#include <assert.h>

#include "max_heap_board.h"

max_heap_board::max_heap_board(uint8_t capacity) {
    this->node_arr = (max_heap_board::node*) malloc(capacity * sizeof(max_heap_board::node));
    this->size = 0;
}

max_heap_board::~max_heap_board() {
    free(this->node_arr);
}

void max_heap_board::push(const board& base_board, const uint8_t& col) {
    base_board.spawn(col, &(this->node_arr[this->size].state));
    this->node_arr[this->size].heuristic = this->node_arr[this->size].state.heuristic_pts();
    this->swim(this->size);
    ++this->size;
}

board max_heap_board::pop() {
    --this->size;
    if(0 != this->size) {
        this->swap(0, this->size);
        this->sink(0);
    }
    return std::move(this->node_arr[this->size].state);
}

void max_heap_board::swap(uint8_t i, uint8_t j) {
    std::swap(this->node_arr[i], this->node_arr[j]);
}

void max_heap_board::swim(uint8_t i) {
    if(0 == i) {
        return;
    }
    uint8_t parent = (i - 1) / 2;
    if(this->node_arr[parent].heuristic < this->node_arr[i].heuristic) {
        this->swap(parent, i);
        this->swim(parent);
    }
}

void max_heap_board::sink(uint8_t i) {
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
