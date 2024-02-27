ResidualSketch: Enhancing Layer Efficiency and Error Reduction in Hierarchical Heavy Hitter Detection with ResNet Innovations
============

Repository structure
--------------------
*  `Common/`: the hash and mmap functions
*  `Struct/`: the data structures, such as heap and hash table*
*  `Algorithm/`: include CocoSketch, USS, SpaceSaving, MVPipe and FullAncestry. Based on SpaceSaving, there are HHH12 and RHHH. Based on CocoSketch and USS, there are Residual-CocoSketch and Residual-USS.

Requirements
-------
- cmake
- g++

How to run
-------
```bash
1. cmake .
2. make
3. ./CPU session-type
- session-type=1:dataset measurement of HH layer distribution
- session-type=2:dataset measurement of numbers of full keys per HH in each layer
- session-type=3:1D-byte Hierarchical Heavy Hitter of various algorithms
- session-type=4:1D-bit Hierarchical Heavy Hitter of various algorithms 
```
