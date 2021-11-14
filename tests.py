from mmap import MMap, Group, Element
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
    assert g_1 ** 3 == Element((800*3)%p, grp), "Exponentiation failed"


def mmap_test():
    p = 3
    grp_cnt = 6
    multimap = MMap(grp_cnt, p)

    assert multimap.get_group(1).id == 1, "1-indexing to 0-indexing"

    assert multimap.get_generator(1) == Element(1, Group(p, 1)), "useful error message"


def encryption_tests():
    enc = encrypt.BitEncryptor(SIMPLE_EC_PROBLEM)
    
    ct = enc.encrypt(1)
    # print(list(map(str, ct)))
    assert enc.decrypt(ct, SIMPLE_EC_PROBLEM_SOL) == 1, "encrypting 1"
    
    ct = enc.encrypt(0)
    assert enc.decrypt(ct, SIMPLE_EC_PROBLEM_SOL) == 0, "encrypting 0"

    
def main():
    group_test()
    element_test()
    mmap_test()
    encryption_tests()
    print("(.づ◡﹏◡)づ. Everything Passed!!!!! sugoi desu-ne~ (.づ◡﹏◡)づ.")




main()