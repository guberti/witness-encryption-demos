from mmap_classes import MMap, Group, Element
from Crypto.Util import number
from sudoku_encryption import SudokuEncryption
from sudoku_reduction import SudokuReduction
from util import SIMPLE_EC_PROBLEM, SIMPLE_EC_PROBLEM_SOL
import encrypt

import sys


def sudoku(input):
    # input textfile is formatted as follow:
    #   first row is a single number n  describing the number of rows in the sudoku
    #   the next n rows consist of n consecutive rows in the set {'.', '1-9'}. '.' denotes that the cell is blank
    def mapchar(c):
        if c == '.':
            return None
        else:
            return int(c)
    
    with open(f"input_puzzles/{input}") as infile:
        n = int(infile.readline())
        puzzle = [list(map(mapchar, infile.readline().strip('\n'))) for _ in range(n)]
        reduction = SudokuReduction(puzzle)
        sets = list(filter(lambda x: len(x)>0, reduction.to_exact_cover_sets()))
        
        with open (f"output_puzzles/sets_{input}", 'w') as outfile:
            for set in sets:
                outfile.write(f'{" ".join(map(str, set))}\n')


def pentomino(input):
    # input textfile is formatted as follow:
    #   first row is a single number n  describing the number of rows in the grid
    #   the next n rows consist of n consecutive rows in the set {'.', '#'}. 
    #       '.' denotes that the cell is blank
    #       '#' denotes that the cell is blocked
    



def main():
    if not len(sys.argv) == 3:
        print ("Usage: python make_sudoku_enc.py problem_type path_to_file")
        return -1

    puzzle_class = sys.argv[1]
    filename = sys.argv[2]
    
    if puzzle_class == 'sudoku':
        sudoku(filename) 




main()