import itertools
from sudoku_reduction import SudokuReduction
from encrypt import BitEncryptor
from mmap_classes import MMap
from Crypto.Util import number

class SudokuEncryption:

    def __init__(self, puzzle):
        assert len(puzzle) == len(puzzle[0]), "not a valid puzzle"
        self.puzzle = puzzle
        self.n = len(puzzle)
        self.bit_enc = None
        self.reducer = None

    def encrypt(self, b: bool):
        # convert puzzle to exact cover
        reducer = SudokuReduction(self.puzzle)
        exact_cover = reducer.to_exact_cover()

        self.bit_enc = BitEncryptor(exact_cover)
        self.reducer = reducer
        return (*self.bit_enc.print_keys(), self.bit_enc.encrypt(b))

    def decrypt(self, ct, solution, mmap):
        # # check that we've setup encryption
        # assert isinstance(self.bit_enc, BitEncryptor)
        # assert len(solution) == len(self.puzzle)
        # assert len(solution[0]) == len(self.puzzle[0])
        # # check that puzzle is a subset of solution
        # for i, j in itertools.product(range(len(solution)), range(len(solution))):
        #     if self.puzzle[i][j] is not None:
        #         assert solution[i][j] == self.puzzle[i][j]
        
        # now convert to set indices
        indices = []
        n = len(solution)
        for i in range(n):
            for j in range(n):
                indices.append(i * n ** 2 + j * n + solution[i][j])
        print(indices)
        total = [0 for _ in range(4 * n ** 2)]
        for i in indices:
            print(self.reducer.collection[i])
            total = list(map(sum, zip(total, self.reducer.collection[i])))
        print(total)
        return BitEncryptor.static_decrypt(ct, indices, mmap)

    @staticmethod
    def static_decrypt(ct, solution, mmap):
        # # check that we've setup encryption
        # assert isinstance(self.bit_enc, BitEncryptor)
        # assert len(solution) == len(self.puzzle)
        # assert len(solution[0]) == len(self.puzzle[0])
        # # check that puzzle is a subset of solution
        # for i, j in itertools.product(range(len(solution)), range(len(solution))):
        #     if self.puzzle[i][j] is not None:
        #         assert solution[i][j] == self.puzzle[i][j]
        
        # now convert to set indices
        indices = []
        n = len(solution)
        for i in range(n):
            for j in range(n):
                indices.append(i * n ** 2 + j * n + solution[i][j])
        
        return BitEncryptor.static_decrypt(ct, indices, mmap)
