# witness-encryption-demos

Puzzle 1 encrypts an unsolvable sudoku puzzle of the following form:
```
..13
....
3...
1...
```

Puzzle 2 encodes the following 9x9 sudoku puzzle:
```
.1.......
.....19.5
.64.2.3..
.4..1....
..13.64..
....7..9.
..5.4.62.
8.92.....
.......5.
```

Puzzle 3 does likewise for the following puzzle:
..48....1
.2..4..3.
6....37..
3....54..
.6..3..9.
..27....8
..31....6
.5..9..4.
4....78..

# Prizes
The prizes are as follows.

For the unsolvable sudoku puzzle:
----"Big" Prize Wallet with 22,700 Satoshi ($11 USD)----
Address: 1MndAbkcPnyYksL3VNJtoNt54vxta4Rpns

For `sudoku1`:
----Small Prize Wallet 1 with 2,700 Satoshi----
Address: 15H79JoED7tZTjmLTojRbiydWEDVMsKxaG

For `sudoku2`:
----Small Prize Wallet 2 with 2,700 Satoshi----
Address: 1LVf6GeNx4DBhMXjy7sUj5bK1jBrRWtmz3

# Converting from Sudoku puzzle to an actual solution

The following mentioned files are located in the `sudoku_sets` director.

For a number in a grid in a Sudoku puzzle solution, we require the row it is in, the column it is in (all counting from the upper left), and the value in the cell. 

For a given cell of the form `(row, col, val)`, where row, col, and val are all zero-indexed, you can find the corresponding set representation in line `row * n ** 2 + col * n + val + 1` (1-indexed lines) of `sudoku_convert.txt`. The number of the encoding of that set is then `row - 1`, where `row` is the row in which that set is now found in the respective `sudoku{n}.txt` file.

The 4x4 sudoku is unsolvable, but we provide the set representations in `unsolvable_sudoku.txt`.

# Format of Encryptions

Bitcoin private keys are generated from 256 bits of randomness. This is done by computing the `sha256` hash twice and then encoding in `base58`. Instructions for doing this can be found online.

For the two large Sudokus with small prizes, 192/256 of the bits are given in the secret key. This is done to speed up encryption and decryption. The other 64 must be obtained through the below mechanism. 

For the unsolvable 4by4 sudoku, all 256 of the bits are encrypted. 

Each puzzle encryption is a `.tar.xz` archive. This contains inside it a public key `pp_l=60.enc` with the standard implemention. 

Using the CLT13 implementation here, a copy of which is located in this repository, multiply together the encodings of the correct sets, subtract it from the `message_bit`, and zero-test it. If it is zero, the bit is a 1. Otherwise, it is a 0. 

We have a sample calculation done in `test_clt_witness_schemes.c` for reference.

This test can be run using `ctest -R "test_clt_witness_scheme" -VV`. To actually perform the encryptions, we ran `ctest -R "compute_clt_witness" -VV`.