import secrets
import random
from Crypto.Util import number
import util
from functools import reduce
from operator import mul
from mmap_classes import MMap, Element
from typing import List

class BitEncryptor:
    # P = 1009 # TODO is this prime large enough?

    def __init__(self, problem: util.ExactCoverProblem):
        self.problem = problem
        self.P = number.getPrime(self.problem.n)
        self.map = MMap(self.problem.n, self.P)

    def print_keys(self):
        return (self.P, self.map)
        
    def bad_rng(self):
        a = secrets.randbits(self.problem.n)
        while a > self.P or a <= 0:
            a = secrets.randbits(self.problem.n)
        return a

    def encrypt(self, b: bool):
        # generator = secrets.SystemRandom()
        # Group Z*_p with 1...(P-1)

        # secrets.choice cannot handle a large range, 
        # so we will use random.randint until a better alternative is found.
        # a = [secrets.choice(group) for _ in range(self.problem.n)]
        a = [self.bad_rng() for _ in range(self.problem.n)]
        ct = []

        g = self.map.get_generator(self.problem.n)
        if b:
            ct.append(g ** reduce(mul, a, 1))
        else:
            ct.append(g ** self.bad_rng())

        for subset in self.problem.subsets:
            g = self.map.get_generator(len(subset))
            ct.append(g ** reduce(mul, [a[i] for i in subset], 1))
            
        return ct

    # testing convenience
    def decrypt(self, ciphertext: List[Element], solution: List[int]):
        assert len(solution) > 0, "this isn't a solution"
        s = reduce(self.map.bilinear_map, [ciphertext[i + 1] for i in solution])
        return s == ciphertext[0]

    @staticmethod
    def static_decrypt(ciphertext: List[Element], solution: List[int], map: MMap):
        assert len(solution) > 0, "this isn't a solution"
        s = reduce(map.bilinear_map, [ciphertext[i + 1] for i in solution])
        return s == ciphertext[0]
