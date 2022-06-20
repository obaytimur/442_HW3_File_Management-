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

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <byteswap.h>

char tempData[512];

// https://www.cs.utah.edu/~germain/PPS/Topics/C_Language/file_IO.html for file input and output operations

struct FAT{
    unsigned fatEntry;
};

struct fileList{
    char fileName[248];
    int firstBlock;
    int fileSize;
};

struct data{
    char content[512];
};

void formatDisk(char *diskName){
    struct FAT fatArray[1];
    struct fileList fileListArray[1];

    FILE *fileObject;
    fileObject = fopen(diskName, "r+");
    fatArray[0].fatEntry = 0xFFFFFFFF;
    memset(&fileListArray[0], 0, sizeof(fileListArray[0].fileName));
    fileListArray[0].fileSize = 0;
    fileListArray[0].firstBlock = 0;
    fseek(fileObject, 0, SEEK_SET);
    fwrite(fatArray, sizeof (fatArray), 1, fileObject);
    fatArray[0].fatEntry = 0;
    for(int i = 1; i< 4096; i++){
        fwrite(fatArray, sizeof(fatArray), 1, fileObject);
    }
    for(int i = 0; i<128; i++){
        fwrite(fileListArray, sizeof(fileListArray), 1, fileObject);
    }
    fclose(fileObject);
}

void writeFile(char *diskName, char *sourceName, char *destName){
    struct FAT fatArray[4096];
    struct fileList fileListArray[128];
    struct data dataContent[4096];
    FILE *fileObject;
    FILE *sourceObject;

    fileObject = fopen(diskName, "r+");
    sourceObject = fopen(sourceName, "r+");

    fread(fatArray, sizeof(fatArray), 1, fileObject);
    fread(fileListArray, sizeof(fileListArray), 1, fileObject);
    fread(dataContent, sizeof(dataContent), 1, fileObject);

    for(int i = 0; i<128; i++){
        if(strcmp(destName, fileListArray[i].fileName) == 0) {
            printf("There is an existing file with this name!\n");
            return;
        }
    }
    int blockOffset = 1;
    int startNumber;
    int endNumber = 1;
    int fileIndex;
    int byteChangedIndex = -1;
    int blockNumber = 0;
    bool isThereSize = false;
    bool isFileFinished = false;
    while(!isFileFinished){
        memset(&tempData[0], 0, sizeof(tempData));
        blockOffset = fread(tempData, sizeof(char), sizeof(tempData), sourceObject);

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

        if(!isThereSize){
            printf("There is not enough size for this file!\n");
            fclose(fileObject);
            return;
        }


        fseek(fileObject, startNumber *sizeof(int), SEEK_SET);
        // https://stackoverflow.com/a/6961239/17761579 for byte exchange
        byteChangedIndex = __bswap_constant_32(startNumber);
        fatArray[startNumber].fatEntry = 0xFFFFFFFF;

        if (blockNumber==0){
            fseek(fileObject, 4096 * sizeof(struct FAT) + 128* sizeof(struct fileList) +startNumber * sizeof(struct data), SEEK_SET);
            fwrite(&tempData, sizeof (char ), blockOffset, fileObject);
            for(int k=0; k<128; k++){
                if(fileListArray[k].firstBlock==0 && fileListArray[k].fileSize==0){
                    fileIndex = k;
                    break;
                }
            }
            strcpy(&fileListArray[fileIndex].fileName[0], destName);
            fileListArray[fileIndex].firstBlock = startNumber;
        }
        else{
            fatArray[endNumber].fatEntry = byteChangedIndex;
            fseek(fileObject, 4096 * sizeof(struct FAT) + 128* sizeof(struct fileList) +startNumber * sizeof(struct data), SEEK_SET);
            fwrite(&tempData, sizeof(char), blockOffset, fileObject);
        }
        endNumber=startNumber;
        blockNumber += 1;
        if(blockOffset!=sizeof(tempData)){
            isFileFinished = true;
        }
    }
    int fileSize = 512 * (blockNumber-1) + blockOffset;
    fileListArray[fileIndex].fileSize = fileSize;

    fseek(fileObject, 0, SEEK_SET);
    fwrite(fatArray, sizeof(fatArray), 1, fileObject);
    fwrite(fileListArray, sizeof(fileListArray), 1, fileObject);
    fclose(fileObject);
    fclose(sourceObject);
}

void readFile(char *diskName, char *sourceName, char *destName){
    struct FAT fatArray[4096];
    struct fileList fileListArray[128];
    struct data dataContent[4096];

    FILE *fileObject;
    FILE *readObject;

    fileObject = fopen(diskName, "r+");
    readObject = fopen(destName, "w+");
    fread(fatArray, sizeof(fatArray), 1, fileObject);
    fread(fileListArray, sizeof(fileListArray), 1, fileObject);
    fread(dataContent, sizeof(dataContent), 1, fileObject);

    bool isFound = false;
    int fileIndex;
    int fileSize;
    for(int i=0; i<128; i++){
        if (!strcmp(sourceName, fileListArray[i].fileName)){
            isFound = true;
            fileIndex = fileListArray[i].firstBlock;
            fileSize = fileListArray[i].fileSize;
            break;
        }
    }
    if(!isFound){
        printf("There is no file with the given name: %s\n", sourceName);
    }

    bool isFinished = false;
    while(!isFinished){
        memset(&tempData[0], 0, sizeof(tempData));
        memcpy(&tempData, &dataContent[fileIndex], sizeof(dataContent[fileIndex]));
        if(fileSize <= 512){
            fwrite(tempData, fileSize * sizeof(char ), 1, readObject);
        }
        else{
            fileSize -= 512;
            fwrite(tempData, 512 * sizeof(char ), 1, readObject);
        }
        if(fatArray[fileIndex].fatEntry == 0xFFFFFFFF){
            //isFinished = true;
            break;
        }
        fileIndex = __bswap_constant_32(fatArray[fileIndex].fatEntry);
    }
    fclose(fileObject);
    fclose(readObject);
}

void deleteFile(char *diskName, char *deleteFileName) {
    struct FAT fatArray[4096];
    struct fileList fileListArray[128];
    struct data dataContent[4096];

    FILE *fileObject;
    fileObject = fopen(diskName, "r+");
    fread(fatArray, sizeof(fatArray), 1, fileObject);
    fread(fileListArray, sizeof(fileListArray), 1, fileObject);
    fread(dataContent, sizeof(dataContent), 1, fileObject);

    int fileIndex;
    bool isFileFound = false;
    for (int i=0; i<128; i++){
        if(!strcmp(deleteFileName, fileListArray[i].fileName)){
            fileIndex = i;
            isFileFound = true;
            break;
        }
    }
    if(!isFileFound){
        printf("There is no such file with the name %s!\n", deleteFileName);
        fclose(fileObject);
        return;
    }
    int dataIndex = fileListArray[fileIndex].firstBlock;
    int changeIndex;
    memset(&fileListArray[fileIndex].fileName[0], 0, sizeof(fileListArray[fileIndex].fileName));
    while(true){
        memset(&dataContent[dataIndex].content[0], 0, sizeof(dataContent[dataIndex].content));
        if(fatArray[dataIndex].fatEntry == 0xFFFFFFFF){
            break;
        }
        changeIndex = __bswap_constant_32(fatArray[dataIndex].fatEntry);
        fatArray[dataIndex].fatEntry = 0;
        dataIndex = changeIndex;
    }
    fatArray[dataIndex].fatEntry = 0;

    fileListArray[fileIndex].firstBlock = 0;
    fileListArray[fileIndex].fileSize = 0;
    fseek(fileObject, 0, SEEK_SET);
    fwrite(fatArray, sizeof(fatArray), 1, fileObject);
    fwrite(fileListArray, sizeof(fileListArray), 1, fileObject);
    fwrite(dataContent, sizeof(dataContent), 1, fileObject);
    fclose(fileObject);
}

void list(char *diskName){
    struct fileList fileArray;
    FILE *fileObject;
    fileObject = fopen(diskName, "r");
    fseek(fileObject, 4096 * sizeof(struct FAT), SEEK_SET);
    printf("File Name: \t File Size:\t File Index:\n");
    for(int i; i<128; i++){
        fread(&fileArray, sizeof(fileArray), 1, fileObject);
        if(fileArray.fileSize != 0 && fileArray.fileName[0] != '.') {
            printf("%s\t %d\t \t%d\n", fileArray.fileName, fileArray.fileSize, fileArray.firstBlock);
        }
    }
    fclose(fileObject);
}

void printFileList(char *diskName){
    struct fileList fileListArray[128];
    FILE *fileObject;
    FILE *writeObject;
    fileObject = fopen(diskName, "r");
    writeObject = fopen("filelist.txt", "w");
    fseek(fileObject, 4096 * sizeof(struct FAT), SEEK_SET);
    fread(fileListArray, sizeof(fileListArray), 1, fileObject);

    fprintf(writeObject, "Item:\tFile name:\t\tFirst Block:\t\tFill size (Bytes)\n");
    for(int i=0; i<128; i++){
        if(!strcmp(fileListArray[i].fileName, "")){
            fprintf(writeObject, "%03d\tNULL\t\t\t%d\t\t\t%d\n", i, fileListArray[i].firstBlock, fileListArray[i].fileSize);
        }
        else{
            fprintf(writeObject, "%03d\t%s\t\t%d\t\t\t%d\n", i, fileListArray[i].fileName, fileListArray[i].firstBlock, fileListArray[i].fileSize);

        }
    }
    fclose(fileObject);
    fclose(writeObject);
}

void printFAT(char *diskName){
    struct FAT fatArray[4096];
    FILE *fileObject;
    FILE *writeObject;
    fileObject = fopen(diskName,"r");
    writeObject = fopen("fat.txt","w");

    fread(fatArray, sizeof(fatArray), 1, fileObject);
    fprintf(writeObject, "Entry \tValue \t\t\tEntry \tValue \t\t\tEntry \tValue \t\t\tEntry \tValue\n");
    for(int i=0; i<4096; i++){
        fprintf(writeObject,"%04d \t0 x %02X %02X %02X %02X \t", i, fatArray[i].fatEntry/(16*16*16*16*16*16), (fatArray[i].fatEntry/(16*16*16*16))%256, (fatArray[i].fatEntry/(256))%256, fatArray[i].fatEntry%256);
        if((i+1)%4 == 0){
            fprintf(writeObject,"\n");
        }
    }
    fclose(fileObject);
    fclose(writeObject);
}

void defragDisk(char *diskName){
    struct FAT fatArray[4096];
    struct FAT defragFatArray[4096];

    struct fileList fileListArray[128];
    struct fileList defragFileListArray[128];

    struct data dataContent[4096];
    struct data defragDataContent[4096];

    FILE *fileObject;
    fileObject = fopen(diskName, "r+");

    fread(fatArray, sizeof(fatArray), 1, fileObject);
    fread(fileListArray, sizeof(fileListArray), 1, fileObject);
    fread(dataContent, sizeof(dataContent), 1, fileObject);

    int defragFileIndex = 0;
    int defragFatIndex = 1;
    int defragStartIndex = 1;
    int defragEndIndex;
    bool isEndBlock = true;
    for(int i=0; i<128; i++){
        if(fileListArray[i].fileSize != 0 && fileListArray[i].firstBlock != 0){
            memcpy(&defragFileListArray[defragFileIndex], &fileListArray[i], sizeof(struct fileList));
            defragFileListArray[defragFileIndex].firstBlock = defragFatIndex;
            defragFileIndex += 1;
            defragEndIndex = defragFileListArray[i].firstBlock;
            while(true){
                memcpy(&defragDataContent[defragFatIndex], &dataContent[defragEndIndex], sizeof(struct data));
                if(isEndBlock){
                    defragFatArray[defragStartIndex].fatEntry = 0xFFFFFFFF;
                    isEndBlock = false;
                }
                else{
                    defragFatArray[defragStartIndex].fatEntry = __bswap_constant_32(defragFatIndex);
                }
                defragStartIndex = defragFatIndex;
                if(fatArray[defragEndIndex].fatEntry == 0xFFFFFFFF){
                    defragFatArray[defragFatIndex].fatEntry = 0xFFFFFFFF;
                    defragFatIndex += 1;
                    isEndBlock = true;
                    break;
                }
                defragFatIndex += 1;
                defragEndIndex = __bswap_constant_32(fatArray[defragEndIndex].fatEntry);
            }
        }
    }
    if(defragFileIndex == 0){
        printf("There is nothing written on the disk that defregmentation can be done!");
    }
    fseek(fileObject, 0, SEEK_SET);
    fwrite(defragFatArray, sizeof(defragFatArray), 1, fileObject);
    fwrite(defragFileListArray, sizeof(defragFileListArray), 1, fileObject);
    fwrite(defragDataContent, sizeof(defragDataContent), 1, fileObject);
    fclose(fileObject);
}

int main(int argc, char *argv[]) {
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
