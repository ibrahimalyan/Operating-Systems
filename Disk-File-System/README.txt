Disk File System
Authored by Ibrahim Alyan
322675356

---Description---
 simulates a basic file system on a virtual disk. It offers functions to format the disk, create, open, close, read, write, rename, and delete files. The code features classes for Inodes and File Descriptors, and a main function acts as a command-line 

---functions---
1-void listAll() : The listAll function iterates through open file descriptors and displays their indices, names, usage status, and sizes. It also prints the content of the simulated disk

2-void fsFormat(int blockSize = 4, int directEntries = 3) : The fsFormat function initializes the file system by formatting the disk with specified block size and direct entry count

3-int CreateFile(string fileName) : The CreateFile function creates a new file in the file system

4-int OpenFile(string FileName ) : The OpenFile function is used to open an existing file in the file system

5- string CloseFile(int fd) : The CloseFile function is used to close a file associated with a given file descriptor (fd)

6- int WriteToFile(int fd, char *buf, int len ) : writes data from a buffer to a simulated disk file  handles various block allocation schemes (direct, single indirect, double indirect)

7-int DelFile( string FileName ) :  provided code deletes a file in a simulated file system removes the file from the directory, and marks the file descriptor as not in use if open

8- int ReadFromFile(int fd, char *buf, int len ) : eads data from a simulated file system efficiently reads blocks, accounting for direct, single indirect, and double indirect blocks

9- int CopyFile(string srcFileName, string destFileName) : copies data from the source to the destination

10 - int RenameFile(string oldFileName, string newFileName) : The RenameFile function is responsible for renaming a file within the file system.

11- AllocateFreeBlock : (int *BitVector, int BitVectorSize) :  Iterate through the BitVector to find a free block and  Mark the block as used by setting it to 1 and  Return the block number (index)

12- GetCurrentBlockNumber(fsInode* fileInode, int blockInUse) :  Helper function to get the current block number for writing

13-  AllocateAndSetBlock(fsInode* fileInode, int blockInUse, int* BitVector, int BitVectorSize)  :Helper function to allocate a new block and update the inode for the direct block 

---Program fiels---
ex1.c    this file contain the function and the main

---How to compile---
compile: g++ stub_code.cpp -o stub_code
run ; ./stub_code
---input---and ---output---direct block
2
4
3
3
a
CreateFile: a with File Descriptor #: 0
6
0
123456789012
1
index: 0: FileName: a , isInUse: 1 file Size: 12
Disk content: '123456789012                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    '

---input---and ---output---renamefile
2
4
3
3
A
CreateFile: A with File Descriptor #: 0
5
0
CloseFile: A with File Descriptor #: 0
10
A
B
1
index: 0: FileName: B , isInUse: 0 file Size: 32759
Disk content: '                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                '"

---input---and ---output--- single block
2
4
3
3
a
CreateFile: a with File Descriptor #: 0
6
0
123456789012345678901234567
1
index: 0: FileName: a , isInUse: 1 file Size: 27
Disk content: '123456789012345678901234567                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 '
7
0
27
ReadFromFile: 123456789012345678901234567
---input---and ---output--- two file using direct and single 
2
4
3
3
a
CreateFile: a with File Descriptor #: 0
6
0
123456789012
7
0
12
ReadFromFile: 123456789012
3
b
CreateFile: b with File Descriptor #: 1
6
1
123456789012345
7
1
15
ReadFromFile: 123456789012345
6
0
123456789012345
7
0
25
ReadFromFile: 1234567890121234567890123
1
index: 0: FileName: a , isInUse: 1 file Size: 27
index: 1: FileName: b , isInUse: 1 file Size: 15
Disk content: '123456789012123456789012   345 	
123456789012345                                                                                                                                                                                                                                                                                                                                                                                                                                                                             '
---input---and ---output--- double block
2
4
3
3
a
CreateFile: a with File Descriptor #: 0
6
0
1234567890123456789012345678901234567890
1
index: 0: FileName: a , isInUse: 1 file Size: 40
Disk content: '1234567890123456789012345678	   
 901234567890                                                                                                                                                                                                                                                                                                                                                                                                                                                                            '
7
0
35
ReadFromFile: 12345678901234567890123456789012345
---input---and ---output--- delet
2
4
3
3
a
CreateFile: a with File Descriptor #: 0
6
0
1234567890123
1
index: 0: FileName: a , isInUse: 1 file Size: 13
Disk content: '123456789012   3                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               '
8
a
DeletedFile: a with File Descriptor #: 0
3
b
CreateFile: b with File Descriptor #: 1
6
1
qwertyuioasdfg
1
index: 0: FileName:  , isInUse: 0 file Size: 0
index: 1: FileName: b , isInUse: 1 file Size: 14
Disk content: 'qwertyuioasd   fg                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              '
---input---and ---output--- copy 
2
4
3
3
a
CreateFile: a with File Descriptor #: 0
6
0
12345
1
index: 0: FileName: a , isInUse: 1 file Size: 5
Disk content: '12345                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           '
5
0
CloseFile: a with File Descriptor #: 0
9
a
b
1
index: 0: FileName: a , isInUse: 0 file Size: 5
index: 1: FileName: b , isInUse: 0 file Size: 8
Disk content: '12345   12345                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   '









