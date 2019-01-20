/*
 * Author: Brighton Ancelin
 *
 * max heap data structure for boards. Ordering done by heuristic points.
 * Should be very fast and relatively compact for move ordering.
 */

#ifndef MAX_HEAP_BOARD_MOVE_H_
#define MAX_HEAP_BOARD_MOVE_H_

#include <cstdint>
#include <utility>

#include "board.h"

class max_heap_board_move {

public:

    /* Constructs an empty max heap with the specified capacity. */
    max_heap_board_move(uint8_t capacity);
    ~max_heap_board_move();
    /* Pushes move col from board base_board onto the max heap. 
     * WARNING: This method does not check for overflow. It is the 
     * responsibility of the programmer to ensure they don't push
     * past capacity.
     * */
    void push(const board& base_board, const uint8_t& col);
    /* Pops the best move from the max heap.
     * WARNING: This method does not check for underflow. It is the
     * responsibility of the programmer to ensure they don't pop
     * past an empty heap.
     * */
    std::pair<board, uint8_t> pop();

private:

    struct node {
        board state;
        uint8_t heuristic;
        uint8_t move;
    };

    /* The array of nodes. */
    node* node_arr;
    /* The size of the heap. */
    uint8_t size;

    /* Swap the two elements and their heuristics. */
    void swap(uint8_t i, uint8_t j);
    /* Have the element swim up the heap. */
    void swim(uint8_t i);
    /* Have the element sink down the heap. */
    void sink(uint8_t i);
};

#endif /* MAX_HEAP_BOARD_H_ */

