# Linear Hashing

University project for algorithms and data structures.

# How does it work?
At the beginning the hash table has two rows (0 and 1 in binary) with each one bucket that has 3 empty spaces.
As soon as there's a collision a bucket will be added and the key will be inserted. Additionally the row at nextToSplit will be split and its contents rehashed.

NextToSplit is incremented by 1. If NextToSplit equals 2^d (meaning all the rows have been split), the d_round is incremented by 1 and nextToSplit is set to zero.


The implementation works as template. It supports a few different methods like insert, erase, size, etc.

