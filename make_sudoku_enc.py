from mmap_classes import MMap, Group, Element
from Crypto.Util import number
from sudoku_encryption import SudokuEncryption
from sudoku_reduction import SudokuReduction
from util import SIMPLE_EC_PROBLEM, SIMPLE_EC_PROBLEM_SOL
import encrypt





def main():
    # puzzle = [[None, 1, None, None, None, None, None, None, None],
    #           [None, None, None, None, None, 1, 9, None, 5],
    #           [None, 6, 4, None, 2, None, 3, None, None],
    #           [None, 4, None, None, 1, None, None, None, None],
    #           [None, None, 1, 3, None, 6, 4, None, None],
    #           [None, None, None, None, 7, None, None, 9, None],
    #           [None, None, 5, None, 4, None, 6, 2, None],
    #           [8, None, 9, 2, None, None, None, None, None],
    #           [None, None, None, None, None, None, None, 5, None]
    #          ]

    puzzle = [[None, None, 4, 8, None, None, None, None, 1],
              [None, 2, None, None, 4, None, None, 3, None],
              [6, None, None, None, None, 3, 7, None, None],
              [3, None, None, None, None, 5, 4, None, None],
              [None, 6, None, None, 3, None, None, 9, None],
              [None, None, 2, 7, None, None, None, None, 8],
              [None, None, 3, 1, None, None, None, None, 6],
              [None, 5, None, None, 9, None, None, 4, None],
              [4, None, None, None, None, 7, 8, None, None]
             ]
    
    reduction = SudokuReduction(puzzle)
    sets = list(filter(lambda x: len(x)>0, reduction.to_exact_cover_sets()))

    for set in sets:
        for elt in set:
            print(elt, end=' ')
        print()



main()