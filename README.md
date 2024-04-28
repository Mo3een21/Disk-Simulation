# Disk-Simulation
This is a unix file system simulator.
Unix File System Simulation
Authored by Abu Katish Moeen

==Description==
This file has 3 main classes: fsInode,FileDescriptor and fsDisk. The fsInode represents an inode where each file should have it's own inode with it's own properties, for example the number of blocks a file uses. The FileDescriptor class has the pair of and inode and a name and acts a normal file descriptor. The fsDisk is the main class where everything comes together in a simulated unix file system. This code uses a block structure of 3 direct blocks, single indirect block and a double indirect block.

Extra Functions:
I only added one extra function which was the allocateNewBlock(), and it's job was to help in allocating blocks for the single and double indirect blocks where the function returns a char of the block to be allocated to be fit as a block pointer.

Main functions:

-fsFormat(): This function formats the disk and initializes the BitVector array and finds how mmany blocks there are depending on the blockSize. After it initialized everything, it sets the isFormated boolean to true.

-CreateFile(): This function takes in a file name as a parameter and opens a new file with that name. It does that by giving it an inode and inserting it to the MainDir map and the OpenFileDescritpors vector.

-OpenFile(): This function's parameter is the file's name is to be opened returns it's file descriptor.It opens the file by setting the inUse boolean to true.

-CloseFile(): This functions sets the inUse boolean to false after taking a the file descriptor.

-WriteToFile(): This function writes to the specified file by the file descriptor.Returns 1 on success, -1 on failure.

-ReadFromFile(): This functions read a specified length of chars from the file specified by the file descriptor given as a parameter. Returns 1 on success, -1 on failure.

-DelFile(): This function deletes all of the blocks and the data written on it for the specified file, it also resets the BitVector blocks to zero so they can be written on them again if called on the write to file function. After that it resets an reinitializes the inode variables to their initial state.

-CopyFile():Copies the contents of file A to file B. If file B is in the disk, we simply override it, but if it doesn't exist, we create it and then copy.

-RenameFile():Renames the file name to a new name.

==Program Files==
stub_code.cpp

==How to compile?==
(on linux terminal)
compile:g++ stub_code.cpp
run:./a.out

==Input==
2
4
3
3
a
3
b
6
0
123456
1



==Output==
CreateFile: a with File Descriptor #: 0
CreateFile: b with File Descriptor #: 1
index: 0: FileName: a , isInUse: 1 file Size: 7
index: 1: FileName: b , isInUse: 1 file Size: 0
Disk content: '123456'
