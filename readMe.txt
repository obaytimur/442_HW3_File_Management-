// Fazlı Oğulcan Baytimur
// EE442 HW3 File System Management

Hello reader, 

This is the read-me file for the code solutions of the third homework of operating systems which is about file system management. The solution consists of bash line commands. To use commands first main file should be compiled with the command "gcc main.c -o 'compileName'". Then the following commands can be used with a disk image named diskName:

1. Format: "./compileName diskName -format" is a function that formats the disk image named ogix
2. Write: "./compileName diskName -write sourceName destinationName" is a function that writes the source file to the disk with the destination name.
3. Read: "./compileName diskName -write sourceName destinationName" is a function that reads the source file from the disk to the folder as the destination name.
4. Delete: "./compileName diskName sourceFile" is a function that deletes the file, provided it is in the disk, from the disk.
5. List: "./compileName diskName -list" is a function that list all the files to the terminal that  are in the disk.
6. Print List: "./compileName diskName -printfilelist" is a function that prints the file list to a text file called "filelist.txt". Their organisations are the same as given in the homework pdf.
7. Print FAT: "./compileName diskName -printfat" is a function that prints the FAT to a text file called "fat.txt". Their organisations are the same as given in the homework pdf.
8. Defragmentation: "./compileName diskName -defragment" is a function that merges files in the FAT continuesly.


Remarks: 
1. Code depends heavily on the characters written in the terminal. However, it is not 100% strict since I want to make it to be used more easily. The commands format, write, read, and list can be executed if their first characters are written correctly. In other words, 
	-f........
	-w........
	-r........
	-l........
will still work. Nonetheless, commands start with the same letter, delete & defregment and print list & print fat, needs to be strictly written to be executed. 	
  
2. In the print list command, commands 6, I have not written the most generic printer functions. Therefore, the alligment of the columns can be changed due to the longness of the the file name. I have written the code assuming file name has charactars between 8-16. Besides this value, allignment can be distorted. However, this does not change correctness. 

3. I think my defragmentation is not working completely correct. It defrags the file list correctly. However, in the fat part it just adds the end file to the end of the previous one. 
