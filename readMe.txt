// Fazlı Oğulcan Baytimur
// EE442 HW3 File System Management

Hello reader, 

This is the read-me file for the code solutions of the third homework of operating systems which is about file system management. The solution consists of bash line commands. To use commands first main file should be compiled with the command "gcc main.c -o 'compileName'". Then the following commands can be used with a disk image named diskName:

1. Format: "./compileName diskName -format" is a function that formats the disk image named ogix
2. Write: "./compileName diskName -write sourceName destinationName" is a function that writes the source file to the disk with the destination name.
3. Read: "./compileName diskName -write sourceName destinationName" is a function that reads the source file from the disk to the folder as the destination name.
4. Delete: "./compileName diskName sourceFile" is a function that deletes the file, provided it is in the disk, from the disk.
5. List: "./compileName diskName -list" is a function that list all the files that  are in the disk.
6. Print List: "./compileName diskName -printfilelist" is a function that prints the file list to a text file called "filelist.txt"
7. Print FAT: "./compileName diskName -printfat" is a function that prints the FAT to a text file called "fat.txt"
8. Defragmentation: "./compileName diskName -defragment" is a function that merges files in the FAT continuesly.

Code depends heavily on the characters written in the terminal. Thus, no typo should be made.  
