import itertools
import math
from util import ExactCoverProblem


class SudokuReduction:
    # row, col, val in 0-(n-1)
    # block num encoding: (row, col, val) -> row * n^2 + col * n + val

    # constraint encoding (4x nxn):
    # nxnblock encoding: (row, col) -> row * n + col
    # row encoding: (row, val) -> row * n + val
    # col encoding: (col, val) -> col * n + val
    # bigblock encoding: (blockrow, blockcol, val) -> (sqrt(n) * blockrow + blockcol) * n + val

    def __init__(self, puzzle):
        # puzzle is n x n of ints from 0 to (n-1)
        # None indicates no value in cell
        self.puzzle = puzzle
        self.n = len(puzzle)
        self.b = int(self.n ** 0.5)
        self.collection = [[0] for _ in range(self.n ** 3)]
        self.gen_sets()
        self.trim_sets()

    def gen_sets(self):
        for row, col, val in itertools.product(range(self.n), range(self.n), range(self.n)):
            offset = self.n ** 2
            # create the row for this thing
            st = [0 for _ in range(4 * self.n ** 2)]

            # one number per cell constraint
            st[self.n * row + col] = 1
            # one of this number per row constraint
            st[offset + row * self.n + val] = 1
            # one of this number over column constraint
            st[offset * 2 + col * self.n + val] = 1
            # one of this number per block constraint
            st[int(offset * 3 + ((row // self.n ** 0.5) * self.n **
                   0.5 + (col // self.n ** 0.5)) * self.n + val)] = 1

            self.collection[row * self.n ** 2 + col * self.n + val] = st

    def trim_sets(self):
        # set those that are invalid (meaning they have a 1 in the same column that a fixed
        # number has a 1) to be empty
        # this means to cover the entry in those spots, they must use the fixed number
        bad_indices = set()
        for row in range(self.n):
            for col in range(self.n):
                if self.puzzle[row][col] is None:
                    continue
                # set every set that shares a 1 in any column with this one to be empty
                # deal with first set, so zero out every other row corresponding to a number in this cell
                # means every block with the same row col values
                bad_indices.update(set(map(lambda v: self.to_index(row, col, v), range(self.n))))
                # second set, same value and same row
                bad_indices.update(set(map(lambda c: self.to_index(row, c, self.puzzle[row][col]), range(self.n))))
                # third set, same value and same column
                bad_indices.update(set(map(lambda r: self.to_index(r, col, self.puzzle[row][col]), range(self.n))))
                # last set, same block
                bad_indices.update(set(map(lambda x: self.to_index((row // self.b) * self.b + x % self.b, (col // self.b) * self.b + x // self.b, self.puzzle[row][col]), range(self.n))))
                # add back the correct set itself
                bad_indices.remove(row * self.n ** 2 + col * self.n + self.puzzle[row][col])
        print(bad_indices)
        # now set all bad_indices to be empty lists
        for i in bad_indices:
            self.collection[i] = [0 for _ in range(4 * self.n ** 2)]

    def to_index(self, row, col, val):
        return int(row * self.n ** 2 + col * self.n + val)

    def to_exact_cover(self):
        def bin_to_numeric(ls):
            return [i for i, b in enumerate(ls) if b]
        subsets = list(map(bin_to_numeric, self.collection))
        return ExactCoverProblem(4 * self.n ** 2, *subsets)

    def __str__(self):
        ml = len(str(self.n))*3+6
        offset = self.n**2

        def str_coords(r, c, v):
            coords = f"({r}, {c}, {v})"
            return coords.rjust(ml, " ")

        def clean(num):
            if num == 1:
                return "1"
            else:
                return " "

        def str_constraints(r, c, v):
            rownum = self.n**2*r + self.n*c + v
            row = '|' + '|'.join([' '.join(map(clean, [(self.collection)[rownum][i+k*offset] for i in range(self.n**2)])) for k in range(4)]) + '|' 
            return row

        header = str_coords("r", "c", "v") + "|" + "cell".ljust(2*offset-1, " ") + "|" + "row".ljust(
            2*offset-1, " ") + "|" + "col".ljust(2*offset-1, " ") + "|" + "block".ljust(2*offset-1, " ") + "|\n"
        return header+"\n".join([str_coords(r, c, v)+str_constraints(r, c, v) for r, c, v in itertools.product(range(self.n), range(self.n), range(self.n))])
