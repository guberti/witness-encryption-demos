from typing import List

class ExactCoverProblem:
    def __init__(n: int, *subsets: List[int]):
        # Validate n
        assert n >= 0
        self.n = n

        # Validate subsets
        for subset in subsets:
            assert min(subset) >= 0
            assert max(subset) < n
        self.subsets = subsets

    def check_witness(indices: List[int]):
        union = set()
        size = 0
        for i in indices:
            union.update(self.subsets[i])
            size += len(self.subsets[i])
        return len(union) == self.n and size == self.n

# Taken from Wikipedia
SIMPLE_EC_PROBLEM = ExactCoverProblem(7,
    [0, 3, 6],
    [0, 3],
    [3, 4, 6],
    [2, 4, 5],
    [1, 2, 5, 6],
    [1, 6]
)

SIMPLE_EC_PROBLEM_SOL = [1, 3, 5]
assert SIMPLE_EC_PROBLEM.check_witness(SIMPLE_EC_PROBLEM_SOL)