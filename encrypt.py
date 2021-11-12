import secrets
from Crypto.Util import number
import util

class BitEncryptor:
    P = 1009 # TODO is this prime large enough?

    def encrypt(b: bool, problem: util.ExactCoverProblem):
        generator = secrets.SystemRandom()
        # Group Z*_p with 1...(P-1)
        group = range(1, BitEncryptor.P)


        a = secrets.sample(group, problem.n)

        def gen_ciphertext(i):
            element =
            return i * i
        ciphertexts = map(gen_ciphertext, range(len(problem.subsets)))


