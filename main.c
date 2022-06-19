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
    for(int i=0; i<128; i++){
        if (!strcmp(sourceName, fileListArray[i].fileName)){
            isFound = true;
            fileIndex = fileListArray[i].firstBlock;
            break;
        }
    }
    if(!isFound){
        printf("There is no file with the given name: %s\n", sourceName);
    }
    int fileSize = fileListArray[fileIndex].fileSize;
    bool isFinished = false;
    int dataIndex = fileListArray[fileIndex].firstBlock;
    while(!isFinished){
        memset(&tempData[0], 0, sizeof(tempData));
        memcpy(&tempData, &dataContent[dataIndex], sizeof(dataContent[dataIndex]));
        if(fileSize <= 512){
            fwrite(tempData, fileSize * sizeof(char ), 1, readObject);
        }
        else{
            fwrite(tempData, fileSize * sizeof(char ), 1, readObject);
            fileSize -= 512;
        }
        if(fatArray[fileIndex].fatEntry == 0xFFFFFFFF){
            isFinished = true;
        }
        fileIndex = __bswap_constant_32(dataIndex);
    }
    fclose(readObject);
    fclose(fileObject);
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
        memset(&dataContent[0], 0, sizeof(dataContent[fileIndex].content));
        if(fatArray[fileIndex].fatEntry == 0xFFFFFFFF){
            break;
        }
        changeIndex = dataIndex;
        dataIndex = __bswap_constant_32(dataIndex);
        fatArray[changeIndex].fatEntry = 0;
    }
    fatArray[dataIndex].fatEntry = 0;

    fileListArray[fileIndex].firstBlock = 0;
    fileListArray[fileIndex].fileSize = 0;
    fseek(fileObject, 0, SEEK_SET);
    fwrite(fatArray, sizeof(fatArray), 1, fileObject);
    fwrite(fileListArray, sizeof(fatArray), 1, fileObject);
    fwrite(dataContent, sizeof(fatArray), 1, fileObject);
    fclose(fileObject);
}

void list(char *diskName){
    struct fileList fileArray;
    FILE *fileObject;
    fileObject = fopen(diskName, "r");
    fseek(fileObject, 4096 * sizeof(struct FAT), SEEK_SET);
    printf("File Name: \t File Size\t File Index\n");
    for(int i; i<128; i++){
        fread(&fileArray, sizeof(fileArray), 1, fileObject);
        if(fileArray.fileSize != 0 && fileArray.fileName[0] != '.') {
            printf("%s\t %d\t \t%d\n", fileArray.fileName, fileArray.fileSize, fileArray.firstBlock);
        }
    }
    fclose(fileObject);
}

void printFileList(char *diskName){

}

void printFAT(char *diskName){
    struct FAT fatArray[4096];
    struct fileList fileListArray[128];
    FILE *fileObject;
    fileObject = fopen(diskName,"r");

    fread(fatArray, sizeof(fatArray), 1, fileObject);
    fread(fileListArray, sizeof(fileListArray), 1, fileObject);
    printf("FAT starts here: %d\n", fatArray[0].fatEntry);

    printf("Address: \t Value:\n");
    for(int i = 0; i<4096; i++){
        printf("%d \t %d\n", i, fatArray[i].fatEntry);
    }
}

void defragDisk(char *diskName){

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
            if(argv[2][2] == 'e') {
                printf("Deleting file from the disk!\n");
                deleteFile(argv[1], argv[3]);
                return 0;
            }
            else if(argv[2][2] == 'i'){
                printf("Dist is being defragmented!\n");
                defragDisk(argv[1]);
                return 0;
            }
        case 'l':
            printf("Listing the Disk!\n");
            list(argv[1]);
            return 0;
        case 'p':
            if (argv[2][7] == 'a'){
                printf("Printing the disk!\n");
                printFAT(argv[1]);
                return 0;
            }
    }
}
