"""
Author: Brighton Ancelin

General playground for testing hypotheses and visualizing things relevant to 
the project. Guaranteed to be messy.
"""

from time import time
import numpy as np
import matplotlib.pyplot as plt

class Board:
    @staticmethod
    def newBoard(rows, cols):
        data = [[None] * rows for _ in range(cols)]
        next = True
        return Board(data, next, ())
                     # 0)
    def __init__(self, data, next, history):
                 # left_heavy):
        self.data = data
        self.next = next
        self.history = history
        # self.left_heavy = left_heavy
    def drop(self, ind):
        i = 0
        while self.data[ind][i] is not None:
            i += 1
            if i >= len(self.data[ind]):
                return None
        new_data = [col.copy() for col in self.data]
        new_data[ind][i] = self.next
        # new_left_heavy = self.left_heavy
        mid = (len(self.data) - 1) / 2
        # if ind < mid:
        #     new_left_heavy += 1
        # elif ind > mid:
        #     new_left_heavy -= 1
        return Board(new_data, not self.next, (*self.history, ind))
                     # new_left_heavy)
    def flip(self):
        new_data = [col.copy() for col in self.data[::-1]]
        # new_left_heavy = -self.left_heavy
        return Board(new_data, self.next, self.history)
                     # new_left_heavy)
    def __str__(self):
        s = ""
        for c in self.data:
            s += "|"
            for elem in c:
                if elem is not None:
                    if elem:
                        s += "T"
                    else:
                        s += "F"
                else:
                    s += " "
            s += "\n"
        return s
    def __eq__(self, other):
        # Assumes other is same size
        for r in range(len(self.data)):
            for c in range(len(self.data[0])):
                if self.data[r][c] != other.data[r][c]:
                    return False
                elif self.data[r][c] is None:
                    break
        return True
    def __hash__(self):
        h = 0
        cols = len(self.data[0])
        for r, row in enumerate(self.data):
            for c, entry in enumerate(row):
                digit = 3 ** ((r * cols) + c)
                if entry is not None:
                    if entry:
                        h += 1 * digit
                    else:
                        h += 2 * digit
        return h


def plot_board_node(board, rows, cols, color_as_terminal=False):
    max_moves = rows * cols
    step = cols ** max_moves
    # step_3size = cols ** (max_moves - len(board.history))
    prev_x = None
    x = 0
    for choice in board.history:
        step /= cols
        prev_x = x
        x += step * choice
    x += step / 2
    y = -len(board.history)
    if color_as_terminal:
        plt.plot(x, y, 'ro')
    else:
        plt.plot(x, y, 'bo')
    if prev_x is not None:
        prev_x += (step * cols) / 2
        plt.plot([prev_x, x], [y + 1, y], 'g-')


def plot_node_lower_links(boards, bins, rows, cols, color='c'):
    max_moves = rows * cols
    N = len(boards)
    assert N == len(bins)
    for bin in range(min(bins), max(bins) + 1):
        members = [boards[i] for i in range(N) if bins[i] == bin]
        if 1 == len(members):
            continue
        print("  bin: {} (size: {})".format(bin, len(members)))
        for member in members:
            print("    {}".format(member.history))
        x = [0] * len(members)
        y = [0] * len(members)
        for i, member in enumerate(members):
            step = cols ** max_moves
            x[i] = 0
            for choice in member.history:
                step /= cols
                x[i] += step * choice
            x[i] += step / 2
            y[i] = -len(member.history)
        x_link = sum(x) / len(x)
        y_link = min(y) - 2
        for i in range(len(members)):
            plt.plot([x[i], x_link], [y[i], y_link], color+'-')


def num_unique_boards(rows, cols, do_plot=False):
    # Returns the number of unique boards, the total number of nodes in the tree
    if do_plot:
        plt.figure()
    b = Board.newBoard(rows, cols)
    agenda = [b]
    visited = set()
    nodes_ct = 0
    while agenda:
        node = agenda.pop(0)
        nodes_ct += 1
        if 0 == (nodes_ct % 10000):
            print("brute: {} nodes explored".format(nodes_ct))
        if do_plot:
            plot_board_node(node, rows, cols)
        visited.add(node)
        for i in range(cols):
            child = node.drop(i)
            if child is None:
                continue
            agenda.append(child)
    return len(visited), nodes_ct


def keep_from_swap_algo(board):
    # Only check last element for swaps
    r_val = board.history[-1]
    between = set()
    i = len(board.history) - 3
    while i >= 0:
        between.add(board.history[i + 1])
        if r_val in between:
            return True
        else:
            l_val = board.history[i]
            if (l_val not in between) and (l_val > r_val):
                return False
            else:
                between.add(l_val)
        i -= 2
    return True


def keep_from_swap_algo_improved(board):
    # Only check last element for swaps

    # Check for singleton swaps
    if 3 <= len(board.history):
        r_val = board.history[-1]
        between = set()
        i = len(board.history) - 3
        while i >= 0:
            between.add(board.history[i + 1])
            if r_val in between:
                break
            else:
                l_val = board.history[i]
                if (l_val not in between) and (l_val > r_val):
                    return False
                else:
                    between.add(l_val)
            i -= 2
    # Check for pair swaps
    if 4 <= len(board.history):
        r_val = board.history[-2:]
        if r_val[0] == r_val[1]:
            r_val = r_val[0]
            between = set()
            i = len(board.history) - 4
            while i >= 0:
                l_val = board.history[i:(i+2)]
                if l_val[0] == l_val[1]:
                    l_val = l_val[0]
                    if (l_val not in between) and (l_val > r_val):
                        return False
                # could be optimized
                between.add(board.history[i])
                between.add(board.history[i+1])
                if r_val in between:
                    break
                i -= 2
    return True


def keep_from_swap_algo_improved_2(board):
    # Only check last element for swaps

    # Check for pair swaps
    if 4 <= len(board.history):
        r_val = board.history[-2:]
        primary_between = set()
        secondary_between = set()
        i = len(board.history) - 4
        while i >= 0:
            l_val = board.history[i:(i+2)]
            if (((l_val[0] not in primary_between) and (l_val[0] != r_val[1])) and \
                ((l_val[1] not in secondary_between) and (l_val[1] != r_val[0]))) and \
                    (l_val[0] > r_val[0]):
                return False
            primary_between.add(l_val[1])
            secondary_between.add(l_val[0])
            if (r_val[0] in primary_between) or \
                    (r_val[1] in secondary_between):
                break
            i -= 2
    # Check for singleton swaps
    if 3 <= len(board.history):
        r_val = board.history[-1]
        between = set()
        i = len(board.history) - 3
        while i >= 0:
            between.add(board.history[i + 1])
            if r_val in between:
                break
            l_val = board.history[i]
            if (l_val not in between) and (l_val > r_val):
                return False
            else:
                between.add(l_val)  # Optimization (assuming every node
                # is checked)
            i -= 2
    return True


def keep_from_insert_algo(board):
    # Only check last element for swaps

    # Check for pair inserts
    if 4 <= len(board.history):
        r_val = board.history[-2:]
        primary_tail = set()
        secondary_tail = set()
        i = len(board.history) - 4
        while i >= 0:
            l_val = board.history[i:(i+2)]
            primary_tail.add(l_val[1])
            secondary_tail.add(l_val[0])
            if (r_val[0] not in primary_tail) and \
                    (r_val[1] not in secondary_tail):
                if l_val[0] > r_val[0]:
                    return False
                elif (l_val[0] == r_val[0]) and (l_val[1] > r_val[1]):
                    return False
            else:
                break
            i -= 2
    if 3 <= len(board.history):
        # Check for singleton inserts
        r_val = board.history[-1]
        tail = set()
        i = len(board.history) - 3
        while i >= 0:
            l_val = board.history[i:(i + 2)]
            if l_val[0] == l_val[1]:
                break
            tail.add(l_val[1])
            if r_val not in tail:
                if l_val[0] > r_val:
                    return False
            else:
                break
            i -= 2
        # Check for singleton swaps
        r_val = board.history[-1]
        between = set()
        i = len(board.history) - 3
        while i >= 0:
            between.add(board.history[i + 1])
            if r_val in between:
                break
            l_val = board.history[i]
            if (l_val not in between) and (l_val > r_val):
                return False
            else:
                between.add(l_val)  # Optimization (assuming every node
                # is checked)
            i -= 2
    return True


def test_swap_algo_unique(rows, cols, do_plot=False):
    # Returns the number of unique boards, the total number of nodes explored
    #  in the tree (using the swap verification algo), the number of
    # duplicate boards not detected by the swap algo
    if do_plot:
        plt.figure()
    b = Board.newBoard(rows, cols)
    agenda = [b]
    if do_plot:
        plot_board_node(b, rows, cols)
    visited = set()
    nodes_ct = 0
    inefficiencies = 0
    while agenda:
        node = agenda.pop(0)
        nodes_ct += 1
        if 0 == (nodes_ct % 10000):
            print("swaps: {} nodes explored".format(nodes_ct))
        vis_len_pre = len(visited)
        visited.add(node)
        if len(visited) != (vis_len_pre + 1):
            inefficiencies += 1
        for i in range(cols):
            child = node.drop(i)
            if (child is None) or (not keep_from_swap_algo_improved(child)):
                if do_plot and (child is not None):
                    plot_board_node(child, rows, cols, True)
                continue
            if do_plot:
                plot_board_node(child, rows, cols)
            agenda.append(child)
    return len(visited), nodes_ct, inefficiencies


def swaps_1(rows, cols):
    # Returns the number of unique boards, the total number of nodes explored
    #  in the tree (using the swap verification algo), the number of
    # duplicate boards not detected by the swap algo
    b = Board.newBoard(rows, cols)
    agenda = [b]
    visited = set()
    nodes_ct = 0
    inefficiencies = 0
    while agenda:
        node = agenda.pop(0)
        nodes_ct += 1
        # if 0 == (nodes_ct % 10000):
        #     print("swaps: {} nodes explored".format(nodes_ct))
        vis_len_pre = len(visited)
        visited.add(node)
        if len(visited) != (vis_len_pre + 1):
            inefficiencies += 1
        for i in range(cols):
            child = node.drop(i)
            if (child is None) or (not keep_from_swap_algo_improved(child)):
                continue
            agenda.append(child)
    return len(visited), nodes_ct, inefficiencies


def swaps_2(rows, cols):
    # Returns the number of unique boards, the total number of nodes explored
    #  in the tree (using the swap verification algo), the number of
    # duplicate boards not detected by the swap algo
    b = Board.newBoard(rows, cols)
    agenda = [b]
    visited = set()
    nodes_ct = 0
    inefficiencies = 0
    while agenda:
        node = agenda.pop(0)
        nodes_ct += 1
        # if 0 == (nodes_ct % 10000):
        #     print("swaps: {} nodes explored".format(nodes_ct))
        vis_len_pre = len(visited)
        visited.add(node)
        if len(visited) != (vis_len_pre + 1):
            inefficiencies += 1
        for i in range(cols):
            child = node.drop(i)
            if (child is None) or (not keep_from_swap_algo_improved_2(child)):
                continue
            agenda.append(child)
    return len(visited), nodes_ct, inefficiencies


def inserts_1(rows, cols):
    # Returns the number of unique boards, the total number of nodes explored
    #  in the tree (using the swap verification algo), the number of
    # duplicate boards not detected by the swap algo
    b = Board.newBoard(rows, cols)
    agenda = [b]
    visited = set()
    nodes_ct = 0
    inefficiencies = 0
    while agenda:
        node = agenda.pop(0)
        nodes_ct += 1
        # if 0 == (nodes_ct % 10000):
        #     print("swaps: {} nodes explored".format(nodes_ct))
        vis_len_pre = len(visited)
        visited.add(node)
        if len(visited) != (vis_len_pre + 1):
            inefficiencies += 1
        for i in range(cols):
            child = node.drop(i)
            if (child is None) or (not keep_from_insert_algo(child)):
                continue
            agenda.append(child)
    return len(visited), nodes_ct, inefficiencies


def get_num_uniq_by_depth(rows, cols, depth, reduction_algo):
    # Wanted to cross reference to https://oeis.org/A212693
    # Wanted to verify my reduction algos were not faulty
    cum_agenda = []
    agenda = [Board.newBoard(rows, cols)]
    num_uniq_by_depth = [0] * depth
    for d in range(depth):
        unset_inds = [ind for ind in range(len(agenda))]
        bins = [-1] * len(agenda)
        next_bin = -1
        while unset_inds:
            next_bin += 1
            i = unset_inds.pop(0)
            bins[i] = next_bin
            assigned_j = []
            for j in unset_inds:
                if agenda[i] == agenda[j]:
                    bins[j] = next_bin
                    assigned_j.append(j)
            for j in assigned_j:
                unset_inds.remove(j)
        num_uniq_by_depth[d] = next_bin + 1
        print("    {}".format(num_uniq_by_depth[d]))
        new_agenda = []
        if d != (depth - 1):
            cum_agenda += agenda
            for node in agenda:
                for col in range(cols):
                    child = node.drop(col)
                    if (child is None) or (not reduction_algo(child)):
                        continue
                    new_agenda.append(child)
            agenda = new_agenda
    return num_uniq_by_depth


def keep_from_refl_algo(board, rows, cols):
    if board.left_heavy > 0:
        return True
    elif board.left_heavy < 0:
        return False
    else:
        if not board.history:
            return True
        # return board.history[-1] < (cols / 2)
        # Quick bad fix
        return True


def test_swap_refl_algo_unique(rows, cols, do_plot=False):
    # Returns the number of unique boards, the total number of nodes explored
    #  in the tree (using the swap-reflection verification algo), the number of
    # duplicate boards not detected by the swap algo
    if do_plot:
        plt.figure()
    b = Board.newBoard(rows, cols)
    agenda = [b]
    if do_plot:
        plot_board_node(b, rows, cols)
    visited = set()
    nodes_ct = 0
    inefficiencies = 0
    while agenda:
        node = agenda.pop(0)
        nodes_ct += 1
        vis_len_pre = len(visited)
        visited.add(node)
        visited.add(node.flip())
        if len(visited) != (vis_len_pre + 1):
            inefficiencies += 1
        for i in range(cols):
            child = node.drop(i)
            if (child is None) or (not keep_from_swap_algo(child)) or (not
                    keep_from_refl_algo(child, rows, cols)):
                if do_plot and (child is not None):
                    plot_board_node(child, rows, cols, True)
                continue
            if do_plot:
                plot_board_node(child, rows, cols)
            agenda.append(child)
    return len(visited), nodes_ct, inefficiencies


def plot_matches_to_depth(rows, cols, depth):
    cum_agenda = []
    agenda = [Board.newBoard(rows, cols)]
    for d in range(depth):
        plt.figure()
        not_unique = [False] * len(agenda)
        unset_inds = [ind for ind in range(len(agenda))]
        bins = [-1] * len(agenda)
        next_bin = -1
        while unset_inds:
            next_bin += 1
            i = unset_inds.pop(0)
            bins[i] = next_bin
            assigned_j = []
            for j in unset_inds:
                if agenda[i] == agenda[j]:
                    bins[j] = next_bin
                    assigned_j.append(j)
                    not_unique[i] = True
                    not_unique[j] = True
            for j in assigned_j:
                unset_inds.remove(j)
        for node in cum_agenda:
            plot_board_node(node, rows, cols)
        for i, node in enumerate(agenda):
            plot_board_node(node, rows, cols, not_unique[i])
        print("depth: {}".format(d))
        plot_node_lower_links(agenda, bins, rows, cols)
        new_agenda = []
        if d != (depth - 1):
            cum_agenda += agenda
            for node in agenda:
                for col in range(cols):
                    child = node.drop(col)
                    if child is None:
                        continue
                    new_agenda.append(child)
            agenda = new_agenda


def verify_matches_by_transform_method(rows, cols, depth):
    cum_agenda = []
    agenda = [Board.newBoard(rows, cols)]
    for d in range(depth):
        plt.figure()
        not_unique = [False] * len(agenda)
        unset_inds = [ind for ind in range(len(agenda))]
        bins = [-1] * len(agenda)
        next_bin = -1
        while unset_inds:
            next_bin += 1
            i = unset_inds.pop(0)
            bins[i] = next_bin
            assigned_j = []
            for j in unset_inds:
                if agenda[i] == agenda[j]:
                    if not do_verify_board_by_transforms(agenda[i]):
                        print("Welp; i: ", agenda[i].history)
                        print(agenda[i])
                    if not do_verify_board_by_transforms(agenda[j]):
                        print("Welp; j: ", agenda[j].history)
                        print(agenda[j])
                    bins[j] = next_bin
                    assigned_j.append(j)
                    not_unique[i] = True
                    not_unique[j] = True
            if not not_unique[i]:
                if do_verify_board_by_transforms(agenda[i]):
                    print("YOYO; j: ", agenda[i].history)
                    print(agenda[i])
            for j in assigned_j:
                unset_inds.remove(j)
        for node in cum_agenda:
            plot_board_node(node, rows, cols)
        for i, node in enumerate(agenda):
            plot_board_node(node, rows, cols, not_unique[i])
        print("depth: {}".format(d))
        plot_node_lower_links(agenda, bins, rows, cols)
        new_agenda = []
        if d != (depth - 1):
            cum_agenda += agenda
            for node in agenda:
                for col in range(cols):
                    child = node.drop(col)
                    if child is None:
                        continue
                    new_agenda.append(child)
            agenda = new_agenda


def do_verify_board_by_transforms(board):
    # Returns true if board is not unique
    # Returns false if board is unique
    hist = board.history
    for i in range(2, len(hist), 2):
        prev = hist[(i-2):i]
        cur = hist[i:(i+1)]
        if CAN_COMMUTE[(prev, cur)]:
            if CAN_COMMUTE[((prev[0],), (prev[1],))]:
                if prev[0] != cur[0]:
                    return True
        if i == (len(hist) - 1):
            break
        else:
            cur = hist[i:(i+2)]
            if CAN_COMMUTE[(prev, cur)]:
                return True
    for i in range(3, len(hist), 2):
        prev = hist[(i-2):i]
        cur = hist[i:(i+1)]
        if CAN_COMMUTE[(prev, cur)]:
            if CAN_COMMUTE[((prev[0],), (prev[1],))]:
                if prev[0] != cur[0]:
                    return True
        if i == (len(hist) - 1):
            break
        else:
            cur = hist[i:(i+2)]
            if CAN_COMMUTE[(prev, cur)]:
                return True
    return False



ROWS = 3
COLS = 4
TRANSFORMS = {}
for p1_move in range(COLS):
    temp = np.eye(COLS + 1, COLS + 1)
    temp[p1_move, p1_move] = 3
    temp[p1_move, -1] = 1
    TRANSFORMS[(p1_move,)] = temp
    for p2_move in range(COLS):
        temp = np.eye(COLS + 1, COLS + 1)
        temp[p2_move, p2_move] = 3
        temp[p2_move, -1] = 2
        TRANSFORMS[(p1_move, p2_move)] = TRANSFORMS[(p1_move,)] @ temp
CAN_COMMUTE = {}
for key_1, val_1 in TRANSFORMS.items():
    for key_2, val_2 in TRANSFORMS.items():
        if (not np.all(val_1 == val_2)) and np.all((val_1 @ val_2) == (val_2 @ \
                val_1)):
            CAN_COMMUTE[(key_1, key_2)] = True
        else:
            CAN_COMMUTE[(key_1, key_2)] = False


if __name__ == '__main__':
    # verify_matches_by_transform_method(ROWS, COLS, 6)
    # plt.show()
    for r in range(6, 7):
        for c in range(7, 8):
            time_start = time()
            DEPTH = 7
            print("({}, {}):".format(r, c))
            print("  starting get_num_uniq_by_depth...")
            num_uniq_by_depth = get_num_uniq_by_depth(r, c, DEPTH,
                                                      keep_from_insert_algo)
            time_get_num = time()
            print("  finished in {} seconds".format(time_get_num - time_start))
            print("  num_uniq_by_depth: {}".format(num_uniq_by_depth))
            # print("  starting get_num_uniq_by_depth (brute)...")
            # num_uniq_by_depth = get_num_uniq_by_depth(r, c, DEPTH, lambda x:
            # True)
            # time_get_num_brute = time()
            # print("  finished in {} seconds".format(time_get_num_brute -
            #                                       time_get_num))
            # print("  num_uniq_by_depth: {}".format(num_uniq_by_depth))

            # time_start = time()
            # print("starting swaps_1...")
            # swaps_1_uniq, swaps_1_nodes_ct, swaps_1_inef = swaps_1(r, c)
            # time_swaps_1 = time()
            # print("finished swaps_1 in {} seconds".format(time_swaps_1 -
            #                                              time_start))
            # print("starting swaps_2...")
            # swaps_2_uniq, swaps_2_nodes_ct, swaps_2_inef = swaps_2(r, c)
            # time_swaps_2 = time()
            # print("finished swaps_2 in {} seconds".format(time_swaps_2 -
            #                                               time_swaps_1))
            # print("starting inserts_1...")
            # inserts_1_uniq, inserts_1_nodes_ct, inserts_1_inef = inserts_1(r, c)
            # time_inserts_1 = time()
            # print("finished inserts_1 in {} seconds".format(time_inserts_1 -
            #                                                 time_swaps_2))
            # print("({}, {}):".format(r, c))
            # print("  swaps_1_uniq: {}".format(swaps_1_uniq))
            # print("  swaps_2_uniq: {}".format(swaps_2_uniq))
            # print("  inserts_1_uniq: {}".format(inserts_1_uniq))
            # print("  swaps_1_nodes_ct: {}".format(swaps_1_nodes_ct))
            # print("  swaps_2_nodes_ct: {}".format(swaps_2_nodes_ct))
            # print("  inserts_1_nodes_ct: {}".format(inserts_1_nodes_ct))
            # print("  swaps_1_inef: {}".format(swaps_1_inef))
            # print("  swaps_2_inef: {}".format(swaps_2_inef))
            # print("  inserts_1_inef: {}".format(inserts_1_inef))
            # time_start = time()
            # print("starting swap...")
            # swap_uniq, reduced_nodes_ct, inef = test_swap_algo_unique(r, c,
            #                                                           False)
            # time_swap = time()
            # print("finished swap in {} seconds".format(time_swap - time_start))
            # print("starting brute...")
            # brute_uniq, total_nodes_ct = num_unique_boards(r, c, False)
            # time_brute = time()
            # print("finished brute in {} seconds".format(time_brute - time_swap))
            # refl_uniq, refl_nodes_ct, inef_refl = test_swap_refl_algo_unique(
            #     r, c, True)
            # print("({}, {}):".format(r, c))
            # print("  format: brute, swap")
            # print("  uniq: {}, {}".format(brute_uniq, swap_uniq))
            # print("  explored: {}, {} (ratio: {:.2f})".format(
            #     total_nodes_ct,
            #     reduced_nodes_ct,
            #     reduced_nodes_ct/total_nodes_ct
            # ))
            # print("({}, {}):".format(r, c))
            # print("  uniq: {}".format(swap_uniq))
            # print("  expl: {}".format(reduced_nodes_ct))
            # print("  dupl: {}".format(inef))
            # print("  %dup: {}".format(inef/reduced_nodes_ct))
            # print()
            # print("  uniq: {}".format(refl_uniq))
            # print("  expl: {}".format(refl_nodes_ct))
            # print("  dupl: {}".format(inef_refl))
            # print("  %dup: {}".format(inef_refl / refl_nodes_ct))
            # plt.show()

