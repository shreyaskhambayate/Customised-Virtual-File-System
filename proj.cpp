#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h> // used for advanced system programming
#include<iostream>
//#include<io.h>

#define MAXINODE 50  // dynamic bcoz we can change this and maxfilesize also
// virtual bcoz sagla ram var chalu ahe
#define READ 1
#define WRITE 2

#define MAXFILESIZE 2048

#define REGULAR 1 // our proj only supports regular file
#define SPECIAL 2  // can be extended in future for other file extensions for special likh rakha he

#define START 0   // lseek saaathi
#define CURRENT 1
#define END 2

typedef struct superblock
{
    int TotalInodes;  //max 50 nodes gheu aapan
    int FreeInode;
}SUPERBLOCK, *PSUPERBLOCK;

typedef struct inode  //94 bytes and tyaachi LL
{
    char FileName[50];
    int InodeNumber;
    int FileSize;
    int FileActualSize;
    int FileType;  // ???
    char *Buffer;
    int LinkCount;
    int ReferenceCount;
    int permission; // 1 23
    struct inode *next;
}INODE, *PINODE, **PPINODE;

typedef struct filetable
{
    int readoffset;  // kuthun read kartay
    int writeoffset; // kuthun write krtay
    int count;   // in future for child .. 8.02pm
    int mode; // 1 2 3
    PINODE ptrinode;
}FILETABLE, *PFILETABLE;

typedef struct ufdt 
{
    PFILETABLE ptrfiletable;
}UFDT;


UFDT UFDTArr[MAXINODE]; //array of structure ufdt, because 50 inodes raahtil
SUPERBLOCK SUPERBLOCKobj;
PINODE head =NULL;  // how come global variable?

void man(char *name)
{
    if(name == NULL) return;

    if(strcmp(name,"create") == 0)
    {
        printf("Description : Used to create new regular file\n");
        printf("Usage : create File_name Permission\n");
    }
    else if(strcmp(name,"write") == 0)
    {
        printf("Description : Used to write data into regular file\n");
        printf("Usage : write File_Name \n");
    }
    else if(strcmp(name,"read") == 0)
    {
        printf("Description : Used to read data from regular file\n");
        printf("Usage : read File_Name No-Of_Bytes_To_Read\n");
    }
    else if(strcmp(name,"ls") == 0)
    {
        printf("Description : Used to list all information of files\n");
        printf("Usage is : ls\n");
    }
    else if(strcmp(name, "stat") == 0)
    {
        printf("Description : Used to display information of file\n");
        printf("Usage : stat File_name\n");
    }
    else if(strcmp(name,"fstat") == 0)
    {
        printf("Description : Used to display information\n");
        printf("Usage : fstat File_Descriptor\n");
    }
    else if(strcmp(name,"truncate") == 0)
    {
        printf("Description : Used to remove data from file\n");
        printf("Usage : truncate File_name\n");
    }
     else if(strcmp(name,"open") == 0)
    {
        printf("Description : Used to open existing file\n");
        printf("Usage : open File_name mode\n");
    }
     else if(strcmp(name,"close") == 0)
    {
        printf("Description : Used to close opened file\n");
        printf("Usage : close File_name mode\n");
    }
     else if(strcmp(name,"closeall") == 0)
    {
        printf("Description : Used to close all opened file\n");
        printf("Usage : closeall\n");
    }
     else if(strcmp(name,"lseek") == 0)
    {
        printf("Description : Used to change file\n");
        printf("Usage : lseek File_name ChangeInOffset StartPoint\n");
    }
    else if(strcmp(name,"rm") == 0)
    {
        printf("Description : Used to delete the file\n");
        printf("Usage : rm File_Name\n");
    }
    else
    {
        printf("ERROR : No manual entry available:");
    }
}

void DisplayHelp()
{
    printf("ls : To List out all files\n");
    printf("clear : To clear console\n");
    printf("open : To open the file\n");
    printf("close : To close the file\n");
    printf("closeall : To close all opened files\n");
    printf("read : To Read the contents from file\n");
    printf("write : To write contents into file\n");
    printf("exit : To terminate file system\n");
    printf("stat : To Display information of file using name\n");
    printf("fstat : To Display information file using file descriptor\n");
    printf("truncate : To Remove all data from file\n");
    printf("rm : To Delete the file\n");
}

int GetFDFromName(char *name)
{
    int i = 0;

    while(i<50)  // search in 50 nodes
    {
        if(UFDTArr[i].ptrfiletable != NULL )
            if( strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName), name) == 0)
                break;

            i++;     
    }

    if (i == 50) return -1;
    else         return i;
}

PINODE Get_Inode(char *name)
{
    PINODE temp = head;   // global variable after struct and it is NULL as i type
    int i = 0;

    if(name == NULL)
        return NULL;

    while(temp != NULL)
    {
        if(strcmp(name,temp->FileName) == 0 )
            break;
        temp = temp->next;
    }    
    return temp;
}

void CreateDILB()  //ya madhe ll tayaar hoil
{
    int i = 1; //inode starts from 1 it is not like array
    PINODE newn = NULL;
    PINODE temp = head;// head is like first from LL

    while(i<= MAXINODE)
    {
            newn = (PINODE)malloc(sizeof(INODE));

        newn->LinkCount = 0;
        newn->ReferenceCount = 0;
        newn->FileType = 0;
        newn->FileSize = 0;

            newn->Buffer = NULL;
            newn->next = NULL;

        newn->InodeNumber = i;

            if(temp == NULL)  // means node is first
            {
                head = newn;
                temp = head;
            }
            else // baaki 49 times code ithe yenaar else madhe
            {
                temp->next = newn;
                temp = temp->next;
            }
            i++;
    }
    printf("DILB created successfully\n");
} //aaplya proj madhe IIT aani DILB are same bcoz our proj is only on ram and not on OS

void InitialiseSuperBlock()
{
    int i = 0;
    while(i<MAXINODE)  // diagram madhe pratyek struct ufdtarr la null kela so that code doesnt crash
    {
        UFDTArr[i].ptrfiletable = NULL;
        i++;
    }

    SUPERBLOCKobj.TotalInodes = MAXINODE;  //ithe superblock structure cha initialsation ahe
    SUPERBLOCKobj.FreeInode = MAXINODE;  // as files get created freenode will get reduced one by one aani delete jhaale file tar freeinode will increase
} // when freeinode becomes 0 we wont be able to create any new file

int CreateFile(char *name, int permission)
{
    int i = 0;
    PINODE temp = head;

    if( (name == NULL) || (permission == 0) || (permission > 3 ) )
        return -1;

    if(SUPERBLOCKobj.FreeInode == 0)
        return -2;

    (SUPERBLOCKobj.FreeInode)--;
    //eg. rumaal chair placeholder 8.21pm multithreading
    if(Get_Inode(name) != NULL)
        return -3;
    
    while(temp != NULL) //used to find empty node 
    {
        if(temp->FileType == 0)  // if filetype 0 node rikaama 
            break;
        temp = temp->next;
    }

    while(i<50)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
            break;
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

    UFDTArr[i].ptrfiletable->count = 1;
    UFDTArr[i].ptrfiletable->mode = permission;
    UFDTArr[i].ptrfiletable->readoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;

    UFDTArr[i].ptrfiletable->ptrinode = temp;

    strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName, name);
    UFDTArr[i].ptrfiletable->ptrinode->FileType = REGULAR;
    UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->LinkCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->FileSize = MAXFILESIZE;
    UFDTArr[i].ptrfiletable->ptrinode->FileActualSize = 0;
    UFDTArr[i].ptrfiletable->ptrinode->permission = permission;  // ithe 0 lihila hota so problem yet hota write read karayla
    UFDTArr[i].ptrfiletable->ptrinode->Buffer = (char *)malloc(MAXFILESIZE);

    return i;  //index
}

// rm_File("Demo.txt")
int rm_File(char * name)
{
    int fd = 0;

    fd = GetFDFromName(name);
    if(fd == -1)
        return -1;

    (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;

    if(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount == 0)
    {
        UFDTArr[fd].ptrfiletable->ptrinode->FileType = 0;
        //free(UFDTArr[fd].ptrfiletable->ptrinode->Buffer);
        free(UFDTArr[fd].ptrfiletable);
    }

    UFDTArr[fd].ptrfiletable = NULL;
    (SUPERBLOCKobj.FreeInode)++;
}

int ReadFile(int fd, char *arr, int isize)  // permission denied
{
    int read_size = 0;

    if(UFDTArr[fd].ptrfiletable == NULL)    return -1;

    if(UFDTArr[fd].ptrfiletable->mode != READ && UFDTArr[fd].ptrfiletable->mode != READ+WRITE)    return -2;

    if((UFDTArr[fd].ptrfiletable->ptrinode->permission != READ && UFDTArr[fd].ptrfiletable->ptrinode->permission != READ+WRITE))  return -2;

    if(UFDTArr[fd].ptrfiletable->readoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)  return -3;

    if(UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)  return -4;

    read_size = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) - (UFDTArr[fd].ptrfiletable->readoffset);
    if(read_size  < isize)
    {
        strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset),read_size);

        UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->readoffset + read_size; 
    }
    else
    {
        strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset),isize  );

        (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + isize;
    }

    return isize;
}

int WriteFile(int fd, char *arr, int isize) // permission denied
{
    if( (  (UFDTArr[fd].ptrfiletable->mode) != WRITE ) && ( (UFDTArr[fd].ptrfiletable->mode  ) != READ+WRITE ) )  return -1;

    if(  ( (UFDTArr[fd].ptrfiletable->ptrinode->permission) != WRITE) && ((UFDTArr[fd].ptrfiletable->ptrinode->permission)!= READ+WRITE) )    return -1;

    if((UFDTArr[fd].ptrfiletable->writeoffset) == MAXFILESIZE )      return -2;

    if((UFDTArr[fd].ptrfiletable->ptrinode->FileType) != REGULAR)    return -3;

    strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->writeoffset),arr,isize  );

    (UFDTArr[fd].ptrfiletable->writeoffset)  = (UFDTArr[fd].ptrfiletable->writeoffset) +isize;

    (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + isize;

    return isize; // write internally returns no of bytes written
}

int OpenFile(char *name, int mode)
{
    int i = 0;
    PINODE temp = NULL;

    if(name == NULL || mode <= 0)
        return -1;

    temp = Get_Inode(name);
    if(temp == NULL)
        return -2;

    if(temp->permission < mode)
        return -3;

    while(i<50)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
            break;
        i++;
    }
    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
    if(UFDTArr[i].ptrfiletable == NULL)  return -1;
    UFDTArr[i].ptrfiletable->count = 1;
    UFDTArr[i].ptrfiletable->mode = mode;
    if(mode == READ + WRITE)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }
    else if(mode == READ)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
    }
    else if(mode == WRITE)
    {
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }
    UFDTArr[i].ptrfiletable->ptrinode = temp;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)++;

    return i;
}

void CloseFileByName(int fd)
{
    UFDTArr[fd].ptrfiletable->readoffset = 0;
    UFDTArr[fd].ptrfiletable->writeoffset = 0;
    (UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount)-- ;
}

int CloseFileByName(char* name)
{
    int i = 0;
    i = GetFDFromName(name);
    if( i == -1)
        return -1;

    UFDTArr[i].ptrfiletable->readoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
    
    return 0;
}

void CloseAllFile()
{
    int i =50;
    while(i<50)
    {
        if(UFDTArr[i].ptrfiletable != NULL)
        {
            UFDTArr[i].ptrfiletable->readoffset = 0;
            UFDTArr[i].ptrfiletable->writeoffset = 0;
            (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
            break;
        }
        i++;
    }
}

int LseekFile(int fd, int size, int from)
{
    if((fd<0) || (from > 2) )  return -1;
    if(UFDTArr[fd].ptrfiletable == NULL)  return -1;

    if((UFDTArr[fd].ptrfiletable->mode == READ)  || (UFDTArr[fd].ptrfiletable->mode == READ+WRITE))
    {
        if(from == CURRENT)
        {
            if(((UFDTArr[fd].ptrfiletable->readoffset) + size) > UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)  return -1;
            if(((UFDTArr[fd].ptrfiletable->readoffset) + size) < 0)  return -1;
            (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + size;
        }
        else if(from == START)
        {
            if(size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))  return -1;
            if(size < 0)  return -1;
            (UFDTArr[fd].ptrfiletable->readoffset)  = size;
        }
        else if(from == END)
        {
            if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size > MAXFILESIZE)  return -1;
            if(((UFDTArr[fd].ptrfiletable->readoffset) + size) < 0)  return -1;
            (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size;
        }
    }
    else if(UFDTArr[fd].ptrfiletable->mode == WRITE)
    {
        if(from == CURRENT)
        {
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + size) > MAXFILESIZE)  return -1;
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + size) < 0)  return -1;
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + size) > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
            (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->writeoffset) + size;
            (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + size;
        }
        else if(from == END)
        {
            if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size > MAXFILESIZE)  return -1;
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + size) < 0)  return -1;
            (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size;
        }
    }
}

void ls_file() // check this
{
    int i = 0;
    PINODE temp = head;

    if(SUPERBLOCKobj.FreeInode == MAXINODE)
    {
        printf("Error : There are no files\n");
        return;
    }

    printf("\nFile Name\tInode number\tFile Size\tLink Count\n");
    printf("------------------------------------------------------\n");
    while(temp != NULL)
    {
        if(temp->FileType != 0)
        {
            printf("%s\t\t%d\t\t%d\t\t%d\n",temp->FileName,temp->InodeNumber,temp->FileActualSize,temp->LinkCount);
        }
        temp = temp->next;
    }
    printf("------------------------------------------------------\n");
}

int fstat_file(int fd) // permission print nahi hot he
{
    PINODE temp = head;
    int i = 0;

    if(fd < 0)  return -1;

    if(UFDTArr[fd].ptrfiletable == NULL)   return -2;

    temp = UFDTArr[fd].ptrfiletable->ptrinode;

    printf("\n---------Statistical Information about file----------\n");
    printf("File Name : %s\n",temp->FileName);
    printf("Inode number %d\n",temp->InodeNumber);
    printf("File size : %d\n", temp->FileSize);
    printf("Actual File Size : %d\n", temp->FileActualSize);
    printf("Link Count : %d\n", temp->LinkCount);
    printf("Reference Count : %d\n", temp->ReferenceCount);

    if(temp->permission == 1)
        printf("File permission : Read only\n");
    else if(temp->permission == 2)
        printf("File permission : Write\n");
    else if(temp->permission == 3)
        printf("File permission : Read & Write\n");
    printf("----------------------------------------------------\n\n");

    return 0;
}

int stat_file(char *name)
{
    PINODE temp = head;
    int i = 0;

    if(name == NULL)  return -1;

    while(temp != NULL)
    {
        if(strcmp(name,temp->FileName)  == 0)
            break;
        temp = temp->next;
    }

    if(temp == NULL)   return -2;

    printf("\n---------Statistical Information about file----------\n");
    printf("File Name : %s\n",temp->FileName);
    printf("Inode number %d\n",temp->InodeNumber);
    printf("File size : %d\n", temp->FileSize);
    printf("Actual File Size : %d\n", temp->FileActualSize);
    printf("Link Count : %d\n", temp->LinkCount);
    printf("Reference Count : %d\n", temp->ReferenceCount);

    if(temp->permission == 1)
        printf("File permission : Read only\n");
    else if(temp->permission == 2)
        printf("File permission : Write\n");
    else if(temp->permission == 3)
        printf("File permission : Read & Write\n");
    printf("----------------------------------------------------\n\n");

    return 0;
}

int truncate_File(char *name)
{
    int fd = GetFDFromName(name);
    if(fd == -1)
        return -1;

    memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer,0,1024);
    UFDTArr[fd].ptrfiletable->readoffset = 0;
    UFDTArr[fd].ptrfiletable->writeoffset = 0;
    UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = 0;
}

int main()
{
    char *ptr = NULL;
    int ret =0, fd = 0, count = 0; //fd for rv of open and create
    char command[4][80], str[80], arr[1024];

    InitialiseSuperBlock();  //automatically initi.. superblock at the start
    CreateDILB();  //these 2 are auxillary data

    while(1)
    {
        fflush(stdin);
        strcpy(str,"");

        printf("\nMarvellous VFS : > ");  //no need

        fgets(str,80,stdin); // scanf("%[^'\n']s",str); //stdin se i/p le 80 bytes aur str me daall

        count = sscanf(str,"%s %s %s %s",command[0],command[1],command[2],command[3]);

            if(count == 1)
            {
                if(strcmp(command[0],"ls") == 0)
                {
                    ls_file();
                }
                else if(strcmp(command[0], "closeall") == 0)
                {
                    CloseAllFile();
                    printf("All files closed successfully\n");
                    continue;
                }
                else if(strcmp(command[0],"clear") == 0)
                {
                    system("cls");
                    continue;
                }
                else if(strcmp(command[0], "help") == 0)
                {
                    DisplayHelp();
                    continue;
                }
                else if(strcmp(command[0], "exit") == 0)
                {
                    printf("Terminating the Marvellous Virtual File System\n");
                    break;
                }
                else
                {
                    printf("\nERROR : Command not found !!!\n");
                    continue;
                }
            }
            else if(count == 2)
            {
                if(strcmp(command[0],"stat") == 0)
                {
                    ret = stat_file(command[1]);
                    //ret = stat_file(atoi(command[1]));
                    if(ret == -1)
                        printf("ERROR : Incorrect paramters\n");
                    if(ret == -2) 
                        printf("ERROR : There is no such file\n");
                    continue;
                }
                else if(strcmp(command[0], "fstat") == 0)
                {
                    ret = fstat_file(atoi(command[1]));
                    if(ret == -1)
                        printf("ERROR : Incorrect paramters\n");
                    if(ret == -2)
                        printf("ERROR : There is no such file\n");
                    continue;
                }
                else if(strcmp(command[0], "close") == 0)
                {
                    ret = CloseFileByName(command[1]);
                    if(ret == -1)
                        printf("ERROR : There is no such file\n");
                    continue;
                }
                else if(strcmp(command[0], "rm") == 0)
                {
                    ret = rm_File(command[1]);
                    if(ret == -1)
                        printf("ERROR : There is no such file\n");
                    continue;
                }
                else if(strcmp(command[0], "man") == 0)
                {
                    man(command[1]);
                }
                else if(strcmp(command[0], "write") == 0)
                {
                    fd = GetFDFromName(command[1]);
                    if(ret == -1)
                    { 
                        printf("ERROR : Incorrect paramter\n");
                        continue;
                    }
                    printf("Enter the data : \n");
                    scanf("%[^\n]",arr);

                    ret = strlen(arr);
                    if(ret == 0)
                    {
                        printf("Error : Incorrect parameter\n");

                        continue;
                    }
                    ret = WriteFile(fd,arr,ret);
                    if(ret == -1)
                        printf("ERROR : Permission denied\n");
                    if(ret == -2)
                        printf("ERROR : There is no sufficient memory to write\n");
                    if(ret == -3)
                        printf("ERROR : It is not a regular file\n");
                }
                else if(strcmp(command[0],"truncate") == 0)
                {
                    ret = truncate_File(command[1]);
                    if(ret == -1)
                        printf("Error : Incorrect paramter\n");
                }
                else
                {
                    printf("\nERROR : Command not found !!!\n");
                    continue;
                }
            }
            else if(count == 3)
            {
                if(strcmp(command[0],"create") == 0)
                {
                    ret = CreateFile(command[1],atoi(command[2]));
                    if(ret >= 0)
                        printf("File is successfully created with file descriptor : %d\n",ret);
                    if(ret == -1)
                        printf("ERROR : Incorrect parameters\n");
                    if(ret == -2)
                        printf("ERROR : There are no inodes\n");
                    if(ret == -3)
                        printf("ERROR : File already exists\n");
                    if(ret == -4)
                        printf("ERROR : Memory allocation failure\n");
                    continue;
                }
                else if(strcmp(command[0], "open") == 0)
                {
                     ret = OpenFile(command[1],atoi(command[2]));
                    if(ret >= 0)
                        printf("File is successfully created/opened with file descriptor : %d\n",ret);
                    if(ret == -1)
                        printf("ERROR : Incorrect parameters\n");
                    if(ret == -2)
                        printf("ERROR : File not present\n");
                    if(ret == -3)
                        printf("Permission denied\n");
                    continue;
                }
                else if(strcmp(command[0],"read") == 0)
                {
                    fd = GetFDFromName(command[1]);
                    if(fd == -1)
                    {
                        printf("Error : Incorrect paramter\n");
                        continue;
                    }
                    ptr = (char*)malloc(sizeof(atoi(command[2])) +1);
                    if(ptr == NULL)
                    {
                        printf("Memory allocation failure\n");
                        continue;
                    }
                    ret = ReadFile(fd,ptr,atoi(command[2]));
                    if(ret == -1)
                        printf("ERROR : File not existing\n");
                    if(ret == -2)
                        printf("ERROR : Permission denied\n");
                    if(ret == -3)
                        printf("ERROR : Reached at the end of file\n");
                    if(ret == -4)
                        printf("ERROR : It is not a regular file\n");
                    if(ret == 0)
                        printf("ERROR : File empty\n");
                    if(ret > 0)
                    {
                        write(2,ptr,ret);
                    }
                    continue;
                }
                else
                {
                    printf("\nERROR : Command not found !!!\n");
                    continue;
                }
            }
            else if(count == 4)
            {
                if(strcmp(command[0],"lseek") == 0)
                {
                    fd = GetFDFromName(command[1]);
                    if(fd == -1)
                    {
                        printf("Error : Incorrect paramter\n");
                        continue;
                    }
                    ret = LseekFile(fd,atoi(command[2]),atoi(command[3]));
                    if(ret == -1)
                    {
                        printf("ERROR : Unable to perform lseek\n");
                    }
                }
                else
                {
                    printf("\nERROR : Command not found\n");
                    continue;
                }
            }
            else
            {
                printf("\nERROR : Command not found !!!\n");
                continue;
            }
    }
    return 0;
}
/*
1. Created a file b.txt and then deleted the same using rm
   Now, if I again try to create b.txt it says that File already exists
   answer = use memset, technically fakt link count kami hota

2. Even after deleting a file using rm, we can still see its info
   using stat filename but not using fstat fd
    But link count 0 display hotoy stat filename kelyavar

    awt swing, event handling, exception handling, multithreading optional
    file handling operations in java-> 
    1. CREATE NEW FILE 2. open an existing file 3. read data from file
    4. write data into the file 5. open the directory/folder and travel all the files
    6. seek operation 
    w3school if internet for 11th hour or javatpoint tutorialspoint
*/