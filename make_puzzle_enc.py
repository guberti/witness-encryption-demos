from mmap_classes import MMap, Group, Element
from Crypto.Util import number
from sudoku_encryption import SudokuEncryption
from sudoku_reduction import SudokuReduction
from pentomino_reduction import PentominoReduction, PentominoBitmasks
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
            return int(c)-1
    
    with open(f"input_puzzles/{input}") as infile:
        n = int(infile.readline())
        puzzle = [list(map(mapchar, infile.readline().strip('\n'))) for _ in range(n)]
        reduction = SudokuReduction(puzzle)
        sets_unfiltered, elems_to_cover = reduction.to_exact_cover_sets()
        sets = list(filter(lambda x: len(x)>0, sets_unfiltered))
        
        # output textfile is formatted as follow:
        #   first row is a single number n, denoting an n element exact cover
        #   second row is a single number S, denoting the number of sets 
        #   subsequent rows are space-separated integers, each row denoting the elements of a set
        with open (f"output_puzzles/sets_{input}", 'w') as outfile:
            outfile.write(f"{elems_to_cover}\n{len(sets)}\n")
            for set in sets:
                outfile.write(f'{" ".join(map(str, set))}\n')


def sudoku_wsol(input):
    def mapchar(c):
        if c == '.':
            return None
        else:
            return int(c)-1
    
    with open(f"input_puzzles/{input}") as infile:
        n = int(infile.readline())
        puzzle = [list(map(mapchar, infile.readline().strip('\n'))) for _ in range(n)]
        reduction = SudokuReduction(puzzle)
        sets_unfiltered, elems_to_cover = reduction.to_exact_cover_sets()
        sets = list(filter(lambda x: len(x)>0, sets_unfiltered))
        
        with open (f"output_puzzles/sets_{input}", 'w') as outfile:
            outfile.write(f"{elems_to_cover}\n{len(sets)}\n")
            for set in sets:
                outfile.write(f'{" ".join(map(str, set))}\n')
        
        puzzle_sol = [list(map(mapchar, infile.readline().strip('\n'))) for _ in range(n)]
        reduction_sol = SudokuReduction(puzzle_sol)
        sets_unfiltered_sol, elems_to_cover_sol = reduction_sol.to_exact_cover_sets()
        sets_sol = list(filter(lambda x: len(x)>0, sets_unfiltered_sol))
        indices = []
        idx = 0
        while len(sets_sol) > 0:
            if sets_sol[0] == sets[idx]:
                indices.append(idx)
                sets_sol.pop(0)
            idx+=1
        with open (f"output_puzzles/sets_{input}", 'a') as outfile:
            outfile.write(" ".join(map(str, indices)))



def pentomino(input):
    # input textfile is formatted as follow:
    #   first row is a single number n  describing the number of rows in the grid
    #   the next n rows consist of n consecutive rows in the set {'.', '#'}. 
    #       '.' denotes that the cell is blank
    #       '#' denotes that the cell is blocked
    def mapchar(c):
        return 1 if c == '.' else 0

    with open(f"input_puzzles/{input}") as infile:
        n = int(infile.readline())
        pieces = list(map(int, infile.readline().split(" ")))
        puzzle = [list(map(mapchar, infile.readline().strip('\n'))) for _ in range(n)]
        reduction = PentominoReduction(puzzle, pieces)
        sets, elems_to_cover = reduction.to_exact_cover_sets()
        ok = PentominoBitmasks()   
        print(len(ok.bitmask_list))
        # reduction.print_cell_map()

        with open (f"output_puzzles/sets_{input}", 'w') as outfile:
            outfile.write(f"{elems_to_cover}\n{len(sets)}\n")
            for set in sets:
                outfile.write(f'{" ".join(map(str, set))}\n')

        # reduction.dump()

        

    



def main():
    if not len(sys.argv) == 3:
        print ("Usage: python make_sudoku_enc.py problem_type path_to_file")
        return -1

    puzzle_class = sys.argv[1]
    filename = sys.argv[2]
    
    if puzzle_class == 'sudoku':
        sudoku(filename) 
    elif puzzle_class == 'sudokuwsol':
        sudoku_wsol(filename)
    elif puzzle_class == 'pento':
        pentomino(filename)




main()