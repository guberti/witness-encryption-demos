import secrets
import util

class BitEncryptor:
    P = 1009 # TODO is this prime large enough?

    def encrypt(b: bool, problem: util.ExactCoverProblem):
        generator = secrets.SystemRandom()
        group = range(1, BitEncryptor.P)

        a = secrets.sample(group, problem.n)

