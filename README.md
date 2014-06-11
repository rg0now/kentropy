kentropy
========

K-order entropy calculator
```
**WARNING:**
**This program is not yet finished, contains bugs and produces invalid results.**

Higher-order entropy calculation is disabled for now, and zero order entropy
is limited to at most 65535 symbols and a simple binary file format.  In
particular, the input file should contain the symbols, each one encoded on 16
bits in network byte order, sequentially. Further file formats and extended
symbol support are under way.

```
Usage: kentropy [options] [file1] [file2]..

Options:
--k NUM    Order of the entropy (default: 0)
--fixrange Scale entropy into [0,1] range
--help     Usage information
--version  Print version

Example 1: kentropy input.txt
Example 2: kentropy --k 4 input.dat
Example 3: kentropy input1.txt input2.txt input3.txt

Written by Andras Majdan, completed by Gabor Retvari
License: GNU General Public License Version 3
Report bugs to <majdan.andras@gmail.com>
