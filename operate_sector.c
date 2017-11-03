#include <sys/types.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <string.h>

#define DISK_NAME "/dev/sda"

/**
 * Initialize the Disk
 */
int initDisk()
{
    int fd = open(DISK_NAME, O_RDWR);

    if(fd < 0){
        perror("Couldn't open device.");
        return 1;
    } 
    return fd;
}
/**
 * Read a set of Blocks into the Buffer
 * Block size is 512bytes
 */
int readFromDisk(int DiskId, char* buffer, int length)
{
    ssize_t readBytes;

    readBytes = read(DiskId, buffer, length);

    FILE *fp;
    fp = fopen("data", "wb");
    fwrite(buffer, 512, 1, fp);
    fclose(fp);
    printf("%d\n", (int)readBytes);
    printf("%#x\n", buffer);
    close(DiskId);

    return 0;
}

/**
 * Write the buffer content to HDD
 */
int writeToDisk(int DiskId, char* buffer, int length)
{
    ssize_t writeBytes;

    writeBytes = write(DiskId, buffer, length);
    printf("%d\n", (int)writeBytes);
    printf("%#x\n", buffer);
    close(DiskId);

    return 0;
}

/**
 * Seek out a block in the Disk
 */
int seekDisk(int DiskId)
{
    printf("Seeking the Disk...\n");
    
    if( lseek(DiskId, 0, SEEK_SET) != 0 ){
        perror("Couldn't seek!\n");
        close(DiskId);
        return 1;
    }
    close(DiskId);
    return 0;
}

int main()
{
    printf("Running the Disk operation tool\n");

    int DiskId;

    char buffer[1024];
    DiskId = initDisk();
    readFromDisk(DiskId, buffer, 512);

    DiskId = initDisk();
    seekDisk(DiskId);

    char wbuffer[440];
    strcpy(wbuffer, "Hello, world!");  

    DiskId = initDisk();
    writeToDisk(DiskId, wbuffer, 440);

    return 0;
}
