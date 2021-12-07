import itertools
import math
from copy import deepcopy 
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
        self.piece_count = sum(pieces)
        self.cart_to_lbl = None
        self.lbl_to_cart = None
        self.tile_bitmasks = PentominoBitmasks().bitmask_list
        self.setcoverrowlen = 0
        # we label the cells from top to bottom, left to right to compress representation. 
        # this requires a mapping from cell coordinates to index
        self.init_cart_to_lbl()

        self.collection = []
        self.gen_sets()


    def init_cart_to_lbl(self):
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
        # print(f"piece {pc_id}")
        # print(self.tile_bitmasks[pc_id])
        for mask in self.tile_bitmasks[pc_id]:
            maskn = len(mask)
            maskm = len(mask[0])

            for r in range(self.n-maskn+1):
                for c in range(self.m-maskm+1):
                    overlap = False
                    thisset = []

                    for i in range (maskn):
                        for j in range (maskm):
                            if mask[i][j] == 0:
                                continue
                            elif mask[i][j] == 1 and self.puzzle[i+r][j+c] == 0:
                                overlap = True
                                break
                            else:
                                thisset.append(self.cart_to_lbl[(i+r, j+c)])
                        if overlap:
                            break
                    
                    if not overlap:
                        sets.append(thisset)
                        # print(thisset)
        # print(sets)
        return sets


    def gen_sets(self):
        ctr = 0
        for id in range(len(self.pieces)):
            templ = self.gen_sets_for_piece(id)
            for _ in range(self.pieces[id]):
                chunk = list(map(lambda x : x+[self.setcoverrowlen + ctr], templ))
                
                self.collection += chunk 
                ctr += 1


    def to_exact_cover_sets(self):
        return self.collection, self.setcoverrowlen + self.piece_count

    def dump(self):
        grid = []
        for r, row in enumerate(self.puzzle):
            out = ""
            for c, cell in enumerate(row):
                if cell == 0:
                    out += '#'
                else:
                    out += '.'
            grid.append(out)
        
        # print ('\n'.join(grid))

        for s in self.collection:
            newgrid = []
            coords = [self.lbl_to_cart[s[i]] for i in range(5)]
            for r in range(self.n):
                out = ""
                for c in range(self.m): 
                    if len(coords) > 0 and coords[0] == (r,c):
                        out += '#'
                        coords.pop(0)
                    else:
                        out += grid[r][c]
                newgrid.append(out)

            print('\n'.join(newgrid))
            print()