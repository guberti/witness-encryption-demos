from mmap_classes import MMap, Group, Element
from Crypto.Util import number
from sudoku_encryption import SudokuEncryption
from sudoku_reduction import SudokuReduction
from util import SIMPLE_EC_PROBLEM, SIMPLE_EC_PROBLEM_SOL
import encrypt


def group_test():
    p = 1009
    id1 = 1
    id2 = 2
    g1 = Group(p, id1)
    g2 = Group(p, id2)

    assert g2.id == 2, "id test fail"
    gen1 = g1.get_generator()
    assert [gen1.group, gen1.value] == [g1, 1], "generator test fail"


def element_test():
    p = 1009
    grp = Group(p, 1)
    badgrp = Group(p, 2)
    g_1 = Element(800, grp)
    g_2 = Element(300, grp)
    g_3 = Element(300, badgrp)

    # basic multiplication test
    assert g_1 * g_2 == Element(91, grp), "Multiplication failed"

    # this should fail because the elts are from groups with different id
    try:
        g_1 * g_3 == Element(91, grp)
        raise TypeError
    except AssertionError:
        pass

    # basic exponentiation operation
    assert g_1 ** 3 == Element((800*3) % p, grp), "Exponentiation failed"


def mmap_test():
    p = 3
    grp_cnt = 6
    multimap = MMap(grp_cnt, p)

    assert multimap.get_group(1).id == 1, "1-indexing to 0-indexing"

    assert multimap.get_generator(1) == Element(
        1, Group(p, 1)), "useful error message"


def encryption_tests():
    # P = number.getPrime(SIMPLE_EC_PROBLEM.n)
    # t_MMap = MMap(SIMPLE_EC_PROBLEM.n, P)
    enc = encrypt.BitEncryptor(SIMPLE_EC_PROBLEM)
    (_, t_MMap) = enc.print_keys()

    ct = enc.encrypt(1)
    print(list(map(str, ct)))
    assert encrypt.BitEncryptor.static_decrypt(ct, SIMPLE_EC_PROBLEM_SOL, t_MMap) == 1, "encrypting 1"

    ct = enc.encrypt(0)
    assert encrypt.BitEncryptor.static_decrypt(ct, SIMPLE_EC_PROBLEM_SOL, t_MMap) == 0, "encrypting 0"


def sudoku_reduction_constraints():
    n = 4
    puzzle = [[None, None, None, 2],
              [None, 0, None, 3],
              [0, None, None, None],
              [1, None, 3, None]]
    reduced = SudokuReduction(puzzle)
    print(reduced)


def sudoku_reduction():
    n = 4
    puzzle = [[None,    None,   None,   2],
              [None,    0,      None,   3],
              [0,       None,   None,   None],
              [1,       None,   3,      None]]

    witness_mat_correct = [
        [3, 1, 0, 2],
        [2, 0, 1, 3],
        [0, 3, 2, 1],
        [1, 2, 3, 0]
    ]

    witness_mat_incorrect = [
        [0, 1, 0, 2],
        [2, 0, 1, 3],
        [0, 3, 2, 1],
        [1, 2, 3, 0]
    ] 

    enc = SudokuEncryption(puzzle)
    p, mmap, ct = enc.encrypt(1)
    assert SudokuEncryption.static_decrypt(ct, witness_mat_correct, mmap) == 1, "encrypting 1"
    try:
        assert SudokuEncryption.static_decrypt(ct, witness_mat_incorrect, mmap) == 1, "encrypting 1"
        raise NameError("incorrect solution should always decrypt to 0")
    except AssertionError:
        pass



def main():
    group_test()
    element_test()
    mmap_test()
    encryption_tests()
    sudoku_reduction_constraints()
    sudoku_reduction()

    print("(.づ◡﹏◡)づ. Everything Passed!!!!! sugoi desu-ne~ (.づ◡﹏◡)づ.")


main()
