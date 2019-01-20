#include <iostream>
#include <cstdint>

#include <chrono>

#include "board.h"
#include "board_insertion_sorter.h"
#include "move_insertion_sorter.h"


/* NOTE: depth must be less than 128 to be able to represent all alpha/beta in range [-127,127] */
int8_t negamax(const board& node, const uint8_t depth, int8_t alpha, int8_t beta, const int8_t color) {
    // There are only two terminal node conditions: winning and tieing. Ties only occur when the board is full
    if(0 == depth) {
        return 0;
    }
    uint8_t child_ct = 0;
    for(uint8_t col = 0; col < board::COLS; ++col) {
        if(node.col_open(col)) {
            if(node.is_winning_move(col)) {
                return depth;
            }
            ++child_ct;
        }
    }
    if(beta > (depth - 2)) {
        beta = depth - 2;
    }
/*
    board_insertion_sorter children_sorted;
    board child;
    uint8_t i = 0;
    uint8_t delta = 0;
    for(uint8_t col = (board::COLS / 2); col < board::COLS;) {
        if(node.col_open(col)) {
            node.spawn(col, &child);
            children_sorted.add(child);
            ++i;
        }
        ++delta;
        if(0 == (delta & 1)) {
            col += delta;
        } else {
            col -= delta;
        }
    }
    for(i = 0; i < children_sorted.size(); ++i) {
        int8_t score = -negamax(children_sorted.get(i), depth-1, -beta, -alpha, -color);
        if(score > alpha) {
            alpha = score;
            if(alpha >= beta) {
                break;
            }
        }
    }
    return alpha;
*/
    board* children = (board*) malloc(child_ct * sizeof(board));
    uint8_t i = 0;
    uint8_t delta = 0;
    for(uint8_t col = (board::COLS / 2); col < board::COLS;) {
        if(node.col_open(col)) {
            node.spawn(col, &(children[i]));
            ++i;
        }
        ++delta;
        if(0 == (delta & 1)) {
            col += delta;
        } else {
            col -= delta;
        }
    }
    for(i = 0; i < child_ct; ++i) {
        int8_t score = -negamax(children[i], depth-1, -beta, -alpha, -color);
        if(score > alpha) {
            alpha = score;
            if(alpha >= beta) {
                break;
            }
        }
    }
    return alpha;
};


/* NOTE: depth must be less than 128 to be able to represent all alpha/beta in range [-127,127] */
uint8_t get_best_move(const board& root, const uint8_t moves_made) {
    auto time_start = std::chrono::high_resolution_clock::now();
    uint8_t depth = (board::ROWS * board::COLS) - moves_made;
    if(0 == depth) {
        // This case should never occur, but if it does, there are no further moves to be made. Return anything.
        return 0;
    }
    uint8_t child_ct = 0;
    for(uint8_t col = 0; col < board::COLS; ++col) {
        if(root.col_open(col)) {
            if(root.is_winning_move(col)) {
                return col;
            }
            ++child_ct;
        }
    }
/*
    move_insertion_sorter children_sorted;
    board child;
    uint8_t i = 0;
    uint8_t delta = 0;
    for(uint8_t col = (board::COLS / 2); col < board::COLS;) {
        if(root.col_open(col)) {
            root.spawn(col, &child);
            children_sorted.add(child, col);
            ++i;
        }
        ++delta;
        if(0 == (delta & 1)) {
            col += delta;
        } else {
            col -= delta;
        }
    }
    int8_t alpha = -depth;
    const int8_t beta = depth - 2; // OLDER: depth - 1
    const int8_t color = 1;
    uint8_t best_move = 0;
    for(i = 0; i < children_sorted.size(); ++i) {
        int8_t score = -negamax(children_sorted.get(i), depth-1, -beta, -alpha, -color);
        if(score > alpha) {
            best_move = children_sorted.get_move_col(i);
            alpha = score;
            if(alpha >= beta) {
                break;
            }
        }
    }
*/
    uint8_t* children_moves = (uint8_t*) malloc(child_ct * sizeof(uint8_t));
    board* children = (board*) malloc(child_ct * sizeof(board));
    uint8_t i = 0;
    uint8_t delta = 0;
    for(uint8_t col = (board::COLS / 2); col < board::COLS;) {
        if(root.col_open(col)) {
            children_moves[i] = col;
            root.spawn(col, &(children[i]));
            ++i;
        }
        ++delta;
        if(0 == (delta & 1)) {
            col += delta;
        } else {
            col -= delta;
        }
    }
    int8_t alpha = -depth;
    const int8_t beta = depth - 2; // OLDER: depth - 1
    const int8_t color = 1;
    uint8_t best_move = 0;
    for(i = 0; i < child_ct; ++i) {
        int8_t score = -negamax(children[i], depth-1, -beta, -alpha, -color);
        if(score > alpha) {
            best_move = children_moves[i];
            alpha = score;
            if(alpha >= beta) {
                break;
            }
        }
    }
    std::cout << "[alpha: " << (int)alpha << "]" << std::endl;
    if(alpha > 0) {
        int abs_turns = (board::ROWS * board::COLS) + 1 - alpha;
        std::cout << "[  Guaranteed win within " << abs_turns << " turns]" << std::endl;
        std::cout << "[  Only " << (abs_turns - moves_made) << " turns left]" << std::endl;
    }
    auto time_end = std::chrono::high_resolution_clock::now();
    std::cout << "[Move evaluation for " << (int)board::ROWS << "x" << (int)board::COLS << " after " << (int)moves_made << " moves was " << std::chrono::duration<double, std::milli>(time_end-time_start).count() << " milliseconds.]" << std::endl;
    return best_move;
};

void comp_vs_human() {
    const uint8_t MAX_MOVES = board::ROWS * board::COLS;
    board game_state = board::blank_board();
    uint8_t move;
    bool victory;
    for(uint8_t moves_made = 0; moves_made < MAX_MOVES; moves_made += 2) {
        // Computer move
        move = get_best_move(game_state, moves_made);
        if(!game_state.col_open(move)) {
            std::cerr << "Code incorrectly returned column " << (int)move << "!" << std::endl;
            break;
        }
        victory = game_state.is_winning_move(move);
        std::cout << "Artetra drops in column " << (int)move << std::endl;
        game_state.drop(move);
        std::cout << game_state;
        if(victory) {
            std::cout << "Victory for Artetra!" << std::endl;
            break;
        }

        if(MAX_MOVES == (moves_made + 1)) {
            break;
        }

        // Human move
        do {
            std::cout << "Which column do you choose? ";
            int temp;
            std::cin >> temp;
            move = (uint8_t)temp;
        } while((move >= board::COLS) || (!game_state.col_open(move)));
        victory = game_state.is_winning_move(move);
        std::cout << "Human drops in column " << (int)move << std::endl;
        game_state.drop(move);
        std::cout << game_state.inverse();
        if(victory) {
            std::cout << "Victory for Human!" << std::endl;
            break;
        }
        
        // Something to show round delimiters
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
    }
};

int main(int argc, char** argv) {
    comp_vs_human();
}
