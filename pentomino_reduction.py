import itertools
import math
from util import ExactCoverProblem



class PentominoBitmasks:
    def __init__(self):
        self.bitmask_list = []
        self.create_bitmasks()

    @staticmethod
    def reflect(tile):
        return [row[::-1] for row in tile]
    
    @staticmethod
    def rotate_ccw(tile):
        n = len(tile)
        m = len(tile[0])
        rotated = [[tile[r][m-1-c] for r in range(n)] for c in range(m)]
        return rotated


    def create_bitmasks(self):
        tiles = [[[1,1,1,1,1]],
                 [[1,1,1], [0,1,1]],
                 [[1,1,1,1], [0,0,0,1]],
                 [[0,1,1], [1,1,0], [0,1,0]],
                 [[1,1,1,0],[0,0,1,1]],
                 [[1,1,1],[0,1,0],[0,1,0]],
                 [[1,0,1], [1,1,1]],
                 [[1,0,0], [1,0,0], [1,1,1]],
                 [[0,0,1], [0,1,1], [1,1,0]],
                 [[0,1,0], [1,1,1], [0,1,0]],
                 [[1,1,1,1], [0,1,0,0]],
                 [[1,1,0], [0,1,0], [0,1,1]]
                 ]

        r = lambda t: self.rotate_ccw(t)
        s = lambda t: self.reflect(t)

        for n, tile in enumerate(tiles):
            if n in [0]:
                tileset = [tile, r(tile)]
            elif n in [5,6,7,8] :
                tileset = [tile, r(tile), r(r(tile)), r(r(r(tile)))]
            elif n in [9]:
                tileset = [tile]
            elif n in [11]:
                tileset = [tile, r(tile), s(tile), r(s(tile))]
            else :
                tileset = [tile, r(tile), r(r(tile)), r(r(r(tile))), s(tile), r(s(tile)), r(r(s(tile))), r(r(r(s(tile))))]
            # print(f"{n} : {len(tileset)}")
            self.bitmask_list.append(tileset)
            
        


class PentominoReduction:
    def __init__(self, puzzle, pieces):
        # puzzle is n x m of ints in {1, 0}
        #   '1' indicates cell is available
        # pieces is a length 12 array containing the number of pieces of each pentomino
        #   [O, P, Q, R, S, T, U, V, W, X, Y ,Z]
        #   See: Conway labeling scheme
        self.puzzle = puzzle
        self.n = len(puzzle)
        self.m = len(puzzle[0])
        self.pieces = pieces
        self.piece_count = len(pieces)
        self.cart_to_lbl = None
        self.lbl_to_cart = None
        self.tile_bitmasks = PentominoBitmasks()
        self.setcoverrowlen = 0
        # we label the cells from top to bottom, left to right to compress representation. 
        # this requires a mapping from cell coordinates to index
        self.create_cell_map()

        self.collection = []


    def create_cell_map(self):
        counter = 0
        cart_to_lbl = dict()
        lbl_to_cart = dict()
        for r, row in enumerate(self.puzzle):
            for c, cell in enumerate(row):
                if cell == 1:
                    cart_to_lbl[(r,c)] = counter
                    lbl_to_cart[counter] = (r,c)
                    counter += 1
        self.cart_to_lbl = cart_to_lbl
        self.lbl_to_cart = lbl_to_cart
        self.setcoverrowlen = len(self.cart_to_lbl)


    def print_cell_map(self):
        for r in range(self.n):
            for c in range(self.m):
                try:
                    print(self.cart_to_lbl[(r,c)] %10, end="")
                except:
                    print('#', end="")
            print()


    def gen_sets_for_piece(self, pc_id):
        sets = []

        for mask in self.tile_bitmasks[pc_id]:
            maskn = len(mask)
            maskm = len(mask[0])

            for (r,c), lbl in self.cart_to_lbl.items():
                if r+maskn < self.n and c+maskm < self.m:
                    overlap = True
                    thisset = []

                    for i in range (maskn):
                        for j in range (maskm):
                            if mask[i][j] == 0:
                                continue
                            elif mask[i][j] == 1 and self.puzzle[i+r][j+c] == 0:
                                overlap = False
                                break
                            else:
                                set.append(self.cart_to_lbl[(i+r, j+c)])
                        if not overlap:
                            thisset = []
                            break
                    
                    sets.append(thisset)
        return sets



    def gen_sets(self):
        for id in range(len(self.cart_to_lbl)):
            self.collection.append()


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
        # print(bad_indices)
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

    def to_exact_cover_sets(self):
        def bin_to_numeric(ls):
            return [i for i, b in enumerate(ls) if b]
        subsets = list(map(bin_to_numeric, self.collection))
        return subsets

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
