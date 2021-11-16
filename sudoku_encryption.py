import itertools
from sudoku_reduction import SudokuReduction
from encrypt import BitEncryptor

class SudokuEncryption:

    def __init__(self, puzzle):
        assert len(puzzle) == len(puzzle[0]), "not a valid puzzle"
        self.puzzle = puzzle
        self.n = len(puzzle)
        self.bit_enc = None

    def encrypt(self, b: bool):
        # convert puzzle to exact cover
        reducer = SudokuReduction(self.puzzle)
        exact_cover = reducer.to_exact_cover()
        self.bit_enc = BitEncryptor(exact_cover)
        return self.bit_enc.encrypt(b)

    def decrypt(self, ct, solution):
        assert isinstance(self.bit_enc, BitEncryptor)
        assert len(solution) == len(self.puzzle)
        assert len(solution[0]) == len(self.puzzle[0])
        # check that puzzle is a subset of solution
        for i, j in itertools.product(range(len(solution)), range(len(solution))):
            if self.puzzle[i][j] is not None:
                assert solution[i][j] == self.puzzle[i][j]
        
        # now convert to set indices
        indices = []
        for i in range(self.n):
            for j in range(self.n):
                indices.append(i * self.n ** 2 + j * self.n + solution[i][j])
        return self.bit_enc.decrypt(ct, indices)
