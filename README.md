# Base-X Encryptor
#### Description:
The Base-X Encryptor Tool is a command-line utility which allows for data to be encrypted and decrypted with two keys (which are files that each contain up to 256 non-duplicate characters).

#### How to Use:
This information is also detailed in bxe --help after compiling.

Usage: bxe `[-e|-d] [-o OUTPUT] [-if INPUTKEY] [-of OUTPUTKEY] [-v] [-f] TARGET`

TARGET: The name of the file to be encrypted/decrypted.

-e: Generates an encrypted file.

-d: Generates a decrypted file.

-o OUTPUT: Provides the name of the output file. If not included, the file itself will be replaced with an encrypted form of its contents.

-v: Activate verbose mode, which provides output to the user.

All flags are case-sensitive.

-f: Does not ask for any further input

To create input/output format key files, one must enter in a series of entirely distinct ASCII characters. Input format keys must contain some permutation of the entire character set that the target file uses (these can be manually created using the ASCII insertion panel in Notepad++), and each key needs to contain at least 2 characters.

#### The Encryption Method:
When this program encrypts data, it breaks the data up into blocks, each of which are first turned into a very large integer (on the order of 10^1000 in some cases), then exported in a different base. While the large integer is being created, the first format key reinterprets the numerical form of each character, and during the base conversion, the second format key determines the mapping between numbers and characters. Once a block is created, a 4-byte header of metadata containing the encrypted block length and the original block length is added to the beginning, to aid decryption.
Since implementing arbitrary-size base conversion gives a complexity on the order of O(n^3), I've decided to limit the size of what could be encoded at a single time. The approximate encryption rate is on the order of a few kilobytes per second on an i5-7500K.
Decryption works in much the same way as encryption does, except that the format keys are swapped.

#### Source Code Information:
##### bxe.c:
bxe.c contains the main function, which handles the UI and part of the encryption process. It also contains several other functions, such as testDuplicates(), which tests for any duplicates within the formatting keys, and nrDisChar(), which is used to compute the number of distinct characters in the targeted file.

##### variableint.c:
This file contains functions that handle the "VariableInt" data structure, which is able to take on integer values of arbitrarily large sizes. Functions include multVarInt(), which is able to multiply VariableInts, and divVarInt(), which is used to divide VariableInts.

##### variableint.h:
This header file contains references to functions from variableint.c that are needed by other parts of the program, along with the definition of the VariableInt struct.

##### encrypt.c:
This file contains encode(), which does the duty of encrypting blocks of data. As this is a relatively complex piece of code, I felt that it would be useful to keep it in its own file.

##### encrypt.h:
This header file contains a reference to encode(), and it defines a struct, lengthString, which is used as the return type of encode().

##### Makefile:
This file stores the compilation instructions for the program. Note that the only two compilation requirements are the MinGW GCC compiler or something equivalent, and GNU Make.