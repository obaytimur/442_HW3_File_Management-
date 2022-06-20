/*
 * Fazlı Oğulcan Baytimur
 * EE442 HW3
 * File System Management
 * Commands:
 * 1. Format
 * 2. Write
 * 3. Read
 * 4. Delete
 * 5. List
 * 6. Print List
 * 7. Print FAT
 * 8. Defragmentation
*/

// Library initializations needed for the functions
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <byteswap.h>

// Character array whose size is 512. Since char has 8 bit size, total would be 512 byte
char tempData[512];

// For the general file operations, input and output, I have used the proposed method whose link is
// commented below. It is simple FILE declaration and fopen method
// https://www.cs.utah.edu/~germain/PPS/Topics/C_Language/file_IO.html [1]


struct FAT{ // FAT struct that has entry method to hold data
    unsigned fatEntry;
};

struct fileList{ // File struct to hold file information: Name, first block number, and file size.
    char fileName[248];
    int firstBlock;
    int fileSize;
};

struct data{ // Data struct to hold data content which has 512 byte chunks
    char content[512];
};

void formatDisk(char *diskName){
    // Struct initializations. They are initialized as an array of size one. The reason is
    // only the first one is going to be set with initial value. The other will be all formatted
    struct FAT fatArray[1];
    struct fileList fileListArray[1];

    // FILE declaration stated at the beginning [1]
    FILE *fileObject;
    fileObject = fopen(diskName, "r+");

    // First entry of the FAT will always be 0 x FF FF FF FF
    fatArray[0].fatEntry = 0xFFFFFFFF;

    // Memory of the first file will also be initialized to zero.
    memset(&fileListArray[0], 0, sizeof(fileListArray[0].fileName));

    // File size and first block will also be zero for the first one
    fileListArray[0].fileSize = 0;
    fileListArray[0].firstBlock = 0;

    // Offset position will be adjusted as proposed in the homework file
    fseek(fileObject, 0, SEEK_SET);

    // Initialized 0 x FF FF FF FF will be written to FAT
    fwrite(fatArray, sizeof (fatArray), 1, fileObject);
    fatArray[0].fatEntry = 0;

    // All the remaining slots will be initialized as zero
    for(int i = 1; i< 4096; i++){
        fwrite(fatArray, sizeof(fatArray), 1, fileObject);
    }

    // File list will also be initialized as zero
    for(int i = 0; i<128; i++){
        fwrite(fileListArray, sizeof(fileListArray), 1, fileObject);
    }

    // file will be closed
    fclose(fileObject);
}

void writeFile(char *diskName, char *sourceName, char *destName){

    // Structs has been initialized with the predefined sizes
    struct FAT fatArray[4096];
    struct fileList fileListArray[128];
    struct data dataContent[4096];

    // FILEs are declared as given in [1]
    FILE *fileObject;
    FILE *sourceObject;

    fileObject = fopen(diskName, "r+");
    sourceObject = fopen(sourceName, "r+");

    // Struct values will be read from the FILE
    fread(fatArray, sizeof(fatArray), 1, fileObject);
    fread(fileListArray, sizeof(fileListArray), 1, fileObject);
    fread(dataContent, sizeof(dataContent), 1, fileObject);

    // File list will be searched for the file with string comparison
    for(int i = 0; i<128; i++){
        // if there is a file with the same name it will not be written
        if(strcmp(destName, fileListArray[i].fileName) == 0) {
            printf("There is an existing file with this name!\n");
            return;
        }
    }

    // variable declarations for the writing loop
    int blockOffset = 1;
    int startNumber;
    int endNumber = 1;
    int fileIndex;
    int byteChangedIndex = -1;
    int blockNumber = 0;
    bool isThereSize = false;
    bool isFileFinished = false;

    // loop will be executed until file writing is finished
    while(!isFileFinished){
        // buffer array will be initialized as zero
        memset(&tempData[0], 0, sizeof(tempData));
        blockOffset = fread(tempData, sizeof(char), sizeof(tempData), sourceObject);

        // first available space will be chosen with its index value
        for(int index = endNumber; index<4096; index++){
            if (fatArray[index].fatEntry == 0){
                isThereSize = true;
                startNumber = index;
                break;
            }
            else{
                isThereSize = false;
            }
        }

        // if there is not a space return nothing
        if(!isThereSize){
            printf("There is not enough size for this file!\n");
            fclose(fileObject);
            return;
        }

        // offset position will be taken
        fseek(fileObject, startNumber *sizeof(int), SEEK_SET);
        // for the byte using method I have found an easy solution. Proposed method
        // converts 8 byte number by two bytes: 1-2-3-4 ---> 4-3-2-1
        // https://stackoverflow.com/a/6961239/17761579 for byte exchange
        byteChangedIndex = __bswap_constant_32(startNumber);
        fatArray[startNumber].fatEntry = 0xFFFFFFFF;

        // write to file list with block chunks,
        if (blockNumber==0){
            // adjust offset
            fseek(fileObject, 4096 * sizeof(struct FAT) + 128* sizeof(struct fileList) +startNumber * sizeof(struct data), SEEK_SET);
            // write the buffered content
            fwrite(&tempData, sizeof (char ), blockOffset, fileObject);
            for(int k=0; k<128; k++){
                // find the available position for the first block
                if(fileListArray[k].firstBlock==0 && fileListArray[k].fileSize==0){
                    fileIndex = k;
                    break;
                }
            }
            // write to the file with the given name
            strcpy(&fileListArray[fileIndex].fileName[0], destName);
            // set the first block number
            fileListArray[fileIndex].firstBlock = startNumber;
        }
        else{
            // fat entry value will be set with the changed endian type
            fatArray[endNumber].fatEntry = byteChangedIndex;
            // same procedure will be done for the remaining blocks to write
            fseek(fileObject, 4096 * sizeof(struct FAT) + 128* sizeof(struct fileList) +startNumber * sizeof(struct data), SEEK_SET);
            fwrite(&tempData, sizeof(char), blockOffset, fileObject);
        }
        // Adjust block numbers
        endNumber=startNumber;
        blockNumber += 1;
        // if size of the buffer is equal to the block offset, it means that it is the last chunk
        if(blockOffset!=sizeof(tempData)){
            // while loop exiter bool
            isFileFinished = true;
        }
    }
    // Total size would be equal to # of complete blocks times 512 bytes
    // plus the offset bytes
    int fileSize = 512 * (blockNumber-1) + blockOffset;
    fileListArray[fileIndex].fileSize = fileSize;

    // offset position is set to zero
    fseek(fileObject, 0, SEEK_SET);
    // Fat and file list is written
    fwrite(fatArray, sizeof(fatArray), 1, fileObject);
    fwrite(fileListArray, sizeof(fileListArray), 1, fileObject);
    // both of them is also closed
    fclose(fileObject);
    fclose(sourceObject);
}

void readFile(char *diskName, char *sourceName, char *destName){
    // struct initializations with the predefined values
    struct FAT fatArray[4096];
    struct fileList fileListArray[128];
    struct data dataContent[4096];

    // FILE declarations as stated above [1]
    FILE *fileObject;
    FILE *readObject;

    fileObject = fopen(diskName, "r+");
    readObject = fopen(destName, "w+");
    // files are read
    fread(fatArray, sizeof(fatArray), 1, fileObject);
    fread(fileListArray, sizeof(fileListArray), 1, fileObject);
    fread(dataContent, sizeof(dataContent), 1, fileObject);

    // parameter initiazlizations
    bool isFound = false;
    int fileIndex;
    int fileSize;

    // Searched for the file name that is going to be read
    for(int i=0; i<128; i++){
        if (!strcmp(sourceName, fileListArray[i].fileName)){
            isFound = true;
            // sets reading files size and block number
            fileIndex = fileListArray[i].firstBlock;
            fileSize = fileListArray[i].fileSize;
            break;
        }
    }
    // if file is not found at the above loop exits function without reading
    if(!isFound){
        printf("There is no file with the given name: %s\n", sourceName);
        return;
    }

    bool isFinished = false;
    // reading file as chunks loop
    while(!isFinished){
        // buffet is set
        memset(&tempData[0], 0, sizeof(tempData));
        memcpy(&tempData, &dataContent[fileIndex], sizeof(dataContent[fileIndex]));

        // if total size of the file less than 512 byte, then read only the remaining parts
        if(fileSize <= 512){
            fwrite(tempData, fileSize * sizeof(char ), 1, readObject);
        }
        // if there is more than 512 byte data, read only the first 512 bytes than decrease total size to be read
        else{
            fileSize -= 512;
            fwrite(tempData, 512 * sizeof(char ), 1, readObject);
        }
        // If end block of file is reached then exit loop
        if(fatArray[fileIndex].fatEntry == 0xFFFFFFFF){
            //isFinished = true;
            break;
        }
        // adjust fat entry value for the right endian
        fileIndex = __bswap_constant_32(fatArray[fileIndex].fatEntry);
    }
    // close FILEs
    fclose(fileObject);
    fclose(readObject);
}

void deleteFile(char *diskName, char *deleteFileName) {
    // Struct initializations with the predefined values
    struct FAT fatArray[4096];
    struct fileList fileListArray[128];
    struct data dataContent[4096];

    // FILE declarations as stated above [1]
    FILE *fileObject;
    fileObject = fopen(diskName, "r+");
    // Read the files from the disk
    fread(fatArray, sizeof(fatArray), 1, fileObject);
    fread(fileListArray, sizeof(fileListArray), 1, fileObject);
    fread(dataContent, sizeof(dataContent), 1, fileObject);

    // Search for the file that is going to be deleted
    int fileIndex;
    bool isFileFound = false;
    for (int i=0; i<128; i++){
        if(!strcmp(deleteFileName, fileListArray[i].fileName)){
            fileIndex = i;
            isFileFound = true;
            break;
        }
    }
    // If it is not found, return function without deletion
    if(!isFileFound){
        printf("There is no such file with the name %s!\n", deleteFileName);
        fclose(fileObject);
        return;
    }
    // set file first block as an index and start deleting from the first block
    int dataIndex = fileListArray[fileIndex].firstBlock;
    int changeIndex;
    // File name is set as zero, i.e. it is deleted
    memset(&fileListArray[fileIndex].fileName[0], 0, sizeof(fileListArray[fileIndex].fileName));
    // content deletion as a loop
    while(true){
        memset(&dataContent[dataIndex].content[0], 0, sizeof(dataContent[dataIndex].content));
        // if last block exit loop
        if(fatArray[dataIndex].fatEntry == 0xFFFFFFFF){
            break;
        }
        // if it is not the end, get the next block from the fat entry value
        changeIndex = __bswap_constant_32(fatArray[dataIndex].fatEntry);
        // clear deleted fat entry value
        fatArray[dataIndex].fatEntry = 0;
        // go to next fat entry value
        dataIndex = changeIndex;
    }
    // Since loop is exited before last entry value is cleared,
    // clear it firstly then continue
    fatArray[dataIndex].fatEntry = 0;

    // delete file infos as weel
    fileListArray[fileIndex].firstBlock = 0;
    fileListArray[fileIndex].fileSize = 0;
    // clear offset
    fseek(fileObject, 0, SEEK_SET);
    // write back to disk
    fwrite(fatArray, sizeof(fatArray), 1, fileObject);
    fwrite(fileListArray, sizeof(fileListArray), 1, fileObject);
    fwrite(dataContent, sizeof(dataContent), 1, fileObject);
    // close disk
    fclose(fileObject);
}

void list(char *diskName){
    // file list array initialization
    struct fileList fileArray;
    // FILE declaration [1]
    FILE *fileObject;
    fileObject = fopen(diskName, "r");
    // offset value is set
    fseek(fileObject, 4096 * sizeof(struct FAT), SEEK_SET);
    // print the title
    printf("File Name: \t File Size:\t File Index:\n");
    // print all the files that are in the file list array
    for(int i; i<128; i++){
        fread(&fileArray, sizeof(fileArray), 1, fileObject);
        if(fileArray.fileSize != 0 && fileArray.fileName[0] != '.') {
            printf("%s\t %d\t \t%d\n", fileArray.fileName, fileArray.fileSize, fileArray.firstBlock);
        }
    }
    // close disk
    fclose(fileObject);
}

void printFileList(char *diskName){
    // Initialize struct for file list array
    struct fileList fileListArray[128];
    // FILEs declarations [1]
    FILE *fileObject;
    FILE *writeObject;
    fileObject = fopen(diskName, "r");
    writeObject = fopen("filelist.txt", "w");
    // offset value is set
    fseek(fileObject, 4096 * sizeof(struct FAT), SEEK_SET);
    // disk is read
    fread(fileListArray, sizeof(fileListArray), 1, fileObject);

    // title is written to the txt file
    fprintf(writeObject, "Item:\tFile name:\t\tFirst Block:\t\tFill size (Bytes)\n");
    for(int i=0; i<128; i++){
        // this if else block is only for tab alignment, since files has a name they require different tabbing than
        // null files
        // Whether there is file or not write the name
        if(!strcmp(fileListArray[i].fileName, "")){
            fprintf(writeObject, "%03d\tNULL\t\t\t%d\t\t\t%d\n", i, fileListArray[i].firstBlock, fileListArray[i].fileSize);
        }
        else{
            fprintf(writeObject, "%03d\t%s\t\t%d\t\t\t%d\n", i, fileListArray[i].fileName, fileListArray[i].firstBlock, fileListArray[i].fileSize);

        }
    }
    // Close FILEs
    fclose(fileObject);
    fclose(writeObject);
}

void printFAT(char *diskName){
    // Initialize fat array
    struct FAT fatArray[4096];
    // FILEs declarations [1]
    FILE *fileObject;
    FILE *writeObject;
    fileObject = fopen(diskName,"r");
    writeObject = fopen("fat.txt","w");

    // disk is read
    fread(fatArray, sizeof(fatArray), 1, fileObject);
    // title is written to the txt file
    fprintf(writeObject, "Entry \tValue \t\t\tEntry \tValue \t\t\tEntry \tValue \t\t\tEntry \tValue\n");
    // Similar method used in the file list printing function is used
    for(int i=0; i<4096; i++){
        // Write fat entry and its corresponding value numbers back to back
        fprintf(writeObject,"%04d \t0 x %02X %02X %02X %02X \t", i, fatArray[i].fatEntry/(16*16*16*16*16*16), (fatArray[i].fatEntry/(16*16*16*16))%256, (fatArray[i].fatEntry/(256))%256, fatArray[i].fatEntry%256);
        if((i+1)%4 == 0){
            // if four of them is written back to back insert a newline here
            fprintf(writeObject,"\n");
        }
    }
    // close Disk
    fclose(fileObject);
    fclose(writeObject);
}

void defragDisk(char *diskName){
    // Two structs are initialized for the old and new defragged ones
    struct FAT fatArray[4096];
    struct FAT defragFatArray[4096];

    struct fileList fileListArray[128];
    struct fileList defragFileListArray[128];

    struct data dataContent[4096];
    struct data defragDataContent[4096];

    // File declaration is done [1]
    FILE *fileObject;
    fileObject = fopen(diskName, "r+");

    // Disk is read
    fread(fatArray, sizeof(fatArray), 1, fileObject);
    fread(fileListArray, sizeof(fileListArray), 1, fileObject);
    fread(dataContent, sizeof(dataContent), 1, fileObject);

    // parameters for the defragmentation indexes are initialized
    int defragFileIndex = 0;
    int defragFatIndex = 1;
    int defragStartIndex = 1;
    int defragEndIndex;
    bool isEndBlock = true;
    for(int i=0; i<128; i++){
        // If there is a file begin defregmentation
        if(fileListArray[i].fileSize != 0 && fileListArray[i].firstBlock != 0){
            // copy the old file to new file list array
            memcpy(&defragFileListArray[defragFileIndex], &fileListArray[i], sizeof(struct fileList));
            // start adjusting indexes
            defragFileListArray[defragFileIndex].firstBlock = defragFatIndex;
            defragFileIndex += 1;
            defragEndIndex = defragFileListArray[i].firstBlock;
            // continue adjusting until file is finished
            while(true){
                memcpy(&defragDataContent[defragFatIndex], &dataContent[defragEndIndex], sizeof(struct data));
                // slide the end block value
                if(isEndBlock){
                    defragFatArray[defragStartIndex].fatEntry = 0xFFFFFFFF;
                    isEndBlock = false;
                }
                else{
                    // convert endians to reach correct block number
                    defragFatArray[defragStartIndex].fatEntry = __bswap_constant_32(defragFatIndex);
                }
                defragStartIndex = defragFatIndex;
                // if end of block is reached in the old fat exit loop
                if(fatArray[defragEndIndex].fatEntry == 0xFFFFFFFF){
                    defragFatArray[defragFatIndex].fatEntry = 0xFFFFFFFF;
                    defragFatIndex += 1;
                    isEndBlock = true;
                    break;
                }
                // if not continue to slide the blocks one by one
                defragFatIndex += 1;
                defragEndIndex = __bswap_constant_32(fatArray[defragEndIndex].fatEntry);
            }
        }
    }
    // if index has not changed defragmentation is not done
    if(defragFileIndex == 0){
        printf("There is nothing written on the disk that defregmentation can be done!");
    }
    // set offset to inital value
    fseek(fileObject, 0, SEEK_SET);
    //write to disk
    fwrite(defragFatArray, sizeof(defragFatArray), 1, fileObject);
    fwrite(defragFileListArray, sizeof(defragFileListArray), 1, fileObject);
    fwrite(defragDataContent, sizeof(defragDataContent), 1, fileObject);
    // close disk
    fclose(fileObject);
}

int main(int argc, char *argv[]) {
    // do the desired function as it is given in the terminal input
    switch (argv[2][1]) {
        case 'f':
            printf("Formatting the disk!\n");
            formatDisk(argv[1]);
            return 0;
        case 'w':
            printf("Writing file to the disk!\n");
            writeFile(argv[1], argv[3], argv[4]);
            return 0;
        case 'r':
            printf("Reading file from the disk!\n");
            readFile(argv[1], argv[3], argv[4]);
            return 0;
        case 'd':
            if(argv[2][3] == 'l') {
                printf("Deleting file from the disk!\n");
                deleteFile(argv[1], argv[3]);
                return 0;
            }
            else if(argv[2][3] == 'f'){
                printf("Disk is being defragmented!\n");
                defragDisk(argv[1]);
                return 0;
            }
        case 'l':
            printf("Listing the Disk!\n");
            list(argv[1]);
            return 0;
        case 'p':
            if(argv[2][7] == 'i'){
                printf("Printing files to the txt file!\n");
                printFileList(argv[1]);
                return 0;
            }
            if (argv[2][7] == 'a'){
                printf("Printing fat format to the txt file!\n");
                printFAT(argv[1]);
                return 0;
            }
    }
}
