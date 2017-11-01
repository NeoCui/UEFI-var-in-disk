#include <sys/types.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <stdio.h>

#define DISK_NAME "/dev/sda"

/**
 * Read a set of Blocks into the Buffer
 * Block size is 512bytes
 */
int readFromDisk(char* buffer, int length)
{
    ssize_t readBytes;
    int fd = open(DISK_NAME, O_RDWR);

    if(fd < 0){
        perror("Couldn't open device.");
        return 1;
    }

    readBytes = read(fd, buffer, length);
    printf("%s\n", buffer);
    close(fd);

    return 0;
}

/**
 * Write the buffer content to HDD
 */
int writeToDisk(char* buffer, int length)
{
    ssize_t writeBytes;
    int fd = open(DISK_NAME, O_RDWR);

    if(fd < 0){
        perror("Couldn't open device.");
        return 1;
    }

    writeBytes = write(fd, buffer, length);
    printf("%s\n", buffer);
    close(fd);

    return 0;
}

/**
 * Seek out a block in the Disk
 */
int seekDisk()
{
    printf("Seeking the Disk...\n");
    int fd = open(DISK_NAME, O_RDWR);
    
    if( lseek(fd, 0, SEEK_SET) != 0 ){
        perror("Couldn't seek!\n");
        close(fd);
        return 1;
    }
    close(fd);
    return 0;
}

int mian()
{
    printf("Running the Disk operation tool\n");

    char buffer[1024];
    readFromDisk(buffer, 512);
    seekDisk();
    writeToDisk(buffer, 300);

    return 0;
}
