# Building Usable Witness Encryption

Witness encryption using multilinear maps was first proposed in 2013, and has continued to evolve since. In this paper, we build on an open-source multilinear map implementation by Carmer and Malozemoff of the graded encoding scheme CLT13 with asymmetric modifications. Using this map, we created the world's first ciphertext encoded with a candidate witness encryption scheme. Finally, using a reduction from Sudoku to Exact Cover, we encrypted the private key to a Bitcoin wallet with 22,700 Satoshi using a Sudoku. 

<p align="center">
  <a href="https://arxiv.org/abs/2112.04581">Read the paper on arXiv</a>
</p>


# What Is This Repository?

To prove that witness encryptions can be done on real puzzles with reasonable security assumptions, we provide three below (along with the code used to generate them in this repository). Each puzzle is a Sudoku, which has been reduced to an equivalent `Exact-Cover` problem. The Sudokus are given in human-readable form below - their reductions are given in the `puzzles/` directory. The underlying CLT13 Multilinear Map implentation was written by [5GenCrypto](https://github.com/5GenCrypto/clt13).

Each puzzle encrypts 256 bits of randomness that were used to generate a Bitcoin wallet. We have transferred 2270 Satoshi (approximately $1.10 at writing) into the wallets that are encrypted by the large Sudokus - puzzles 2 and 3. These Sudokus can be solved by hand or with a computerized solver, and are given only as proof of the validity and scalability of our scheme.

We also created a `4x4` Sudoku which has no satisfying solutuion. We used this Sudoku to encrypt a wallet containing 22,700 Satoshi (approximately $11 at writing), as the only way this money can be retrieved is through defeating our scheme.

# The Puzzles

Sudoku 1 encodes the following 9x9 sudoku puzzle:
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

Sudoku 2 does likewise for the following puzzle:
```
..48....1
.2..4..3.
6....37..
3....54..
.6..3..9.
..27....8
..31....6
.5..9..4.
4....78..
```

The unsolvable Sudoku corresponds to the following puzzle:
```
..13
....
3...
1...
```

# Encryption Statistics

All encryptions were done on a Dell Precision 7540 Mobile Workstation with 72 GB of RAM and an Intel Core i9-9880H CPU @ 2.30GHz. 

- `sudoku1` was encrypted with security parameter `lambda = 20` and multi-linearity `kappa=729`. Its exact cover reduction has 248 sets. It took ~40 minutes to encrypt and has a ciphertext size of 934 MB.
- `sudoku2` was encrypted with security parameter `lambda = 30` and multi-linearity `kappa=729`. Its exact cover reduction has 206 sets. It took ~3 hours to encrypt and has a ciphtertext size of 4.1 GB.
- `unsolvable` was encrypted with security parameter `lambda = 60` and multi-linearit `kappa=32`. Its exact cover reduction has 32 sets. It took ~4 hours to encrypt and has a ciphertext size of 10.6 GB.

# Prize Public Keys

To check if the prizes have been claimed, [Blockchain Explorer](https://www.blockchain.com/explorer) can be used (see below links).

For `sudoku1`:
----Small Prize Wallet 1 with 2,700 Satoshi----
Address: **[15H79JoED7tZTjmLTojRbiydWEDVMsKxaG](https://www.blockchain.com/btc/address/15H79JoED7tZTjmLTojRbiydWEDVMsKxaG)**

For `sudoku2`:
----Small Prize Wallet 2 with 2,700 Satoshi----
Address: **[1LVf6GeNx4DBhMXjy7sUj5bK1jBrRWtmz3](https://www.blockchain.com/btc/address/1LVf6GeNx4DBhMXjy7sUj5bK1jBrRWtmz3)**

For the unsolvable sudoku puzzle:
----"Big" Prize Wallet with 22,700 Satoshi ($11 USD)----
Address: **[1MndAbkcPnyYksL3VNJtoNt54vxta4Rpns](https://www.blockchain.com/btc/address/1MndAbkcPnyYksL3VNJtoNt54vxta4Rpns)**

# Converting from Sudoku puzzle to an actual solution

The following mentioned files are located in the `sudoku_sets` director.

For a number in a grid in a Sudoku puzzle solution, we require the row it is in, the column it is in (all counting from the upper left), and the value in the cell. 

For a given cell of the form `(row, col, val)`, where row, col, and val are all zero-indexed, you can find the corresponding set representation in line `row * n ** 2 + col * n + val + 1` (1-indexed lines) of `sudoku_convert.txt`. The number of the encoding of that set is then `row - 1`, where `row` is the row in which that set is now found in the respective `sudoku{n}.txt` file.

The 4x4 sudoku is unsolvable, but we provide the set representations in `unsolvable_sudoku.txt`.

# Downloading and Decoding the Encryptions

Because of file size limits on GitHub, we've uploaded the puzzle encryptions to Google Drive. They may be accessed via this link:

<p align="center">
  <a href="https://drive.google.com/drive/folders/1-pnitOx51YKRgZ_uDzMWnV4RouNcjwg-?usp=sharing">Download the ciphertexts</a>
</p>

This folder contains the subset reductions of the above Sudoku problems, as well as `.tgz` archives that are the actual encryptions. Each `.tgz` file has the following structure:

```
sudoku1
├───pp_l=x.enc
├───message_bit_0
│   ├───message_bit.enc
│   ├───set_0.enc
│   ├───set_1.enc
│   ...
│   └───set_n.enc
├───message_bit_1
│   ├───message_bit.enc
│   ├───set_0.enc
│   ├───set_1.enc
│   ...
│   └───set_n.enc
...
```

For each puzzle, the public key is shared between all bit encryptions. Once an exact cover has been calculated (say, for example, with sets `l, m, n, p`), we can decrypt the bits. To decrypt bit `k`, use the sets from the folder `message_bit_k`. Compute:

```
is_zero(set_l * set_m * set_n * set_p - message_bit)
```

The boolean returned by `is_zero` is our bit `k`. Repeating this process for all 256 bits will yield the randomness used to generate our Bitcoin private keys.

# Accessing the Prizes

Bitcoin private keys are generated from 256 bits of randomness. This is done by computing the `sha256` hash twice and then encoding in `base58`. The below Python snippet turns a `bytes` object with 256 bits into a private key:

```python
def get_private_key(input_bytes):
    padded = b"\x80" + input_bytes
    hashed = sha256(sha256(padded))
    return b58(padded + hashed[:4])
```

The 256 bits we used to generate the private keys for each wallet have been encrypted. Once they have been retreived as described above, this mechanism should be used to convert them to a private key that can be used to transfer the funds elsewhere. Further details may be found on [https://strm.sh/post/bitcoin-address-generation/](https://strm.sh/post/bitcoin-address-generation/).

# Decryption Code Snippet

We provide a unit test that performs both encryption and decryption in `test_clt_witness_schemes.c`. The test can be run using `ctest -R "test_clt_witness_scheme"`.

The code we used to generate these encryptions may be found in `test/compute_clt_witness.c`.
