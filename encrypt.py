import secrets
from Crypto.Util import number
import util
from functools import reduce
from operator import mul
from mmap import MMap, Element
from typing import List

class BitEncryptor:
    P = 1009 # TODO is this prime large enough?

    def __init__(self, problem: util.ExactCoverProblem):
        self.problem = problem
        self.map = MMap(problem.n, BitEncryptor.P)

    def encrypt(self, b: bool):
        # generator = secrets.SystemRandom()
        # Group Z*_p with 1...(P-1)
        group = range(1, BitEncryptor.P)

        a = [secrets.choice(group) for _ in range(self.problem.n)]
        ct = []

        g = self.map.get_generator(self.problem.n)
        if b:
            ct.append(g ** reduce(mul, a, 1))
        else:
            ct.append(g ** secrets.choice(group))

        for subset in self.problem.subsets:
            g = self.map.get_generator(len(subset))
            ct.append(g ** reduce(mul, [a[i] for i in subset], 1))
            
        return ct

    def decrypt(self, ciphertext: List[Element], solution: List[int]):
        assert len(solution) > 0, "this isn't a solution"
        s = reduce(self.map.bilinear_map, [ciphertext[i + 1] for i in solution])
        return s == ciphertext[0]

