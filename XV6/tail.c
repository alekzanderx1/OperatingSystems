#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char buf[512];

void tail(int fd, int linesToRead) {
    int n,i;
    int totalLines = 0;

    // create a temporary file to store the contents of input since it could either be from pipe or file
    // not using a buffer array to store lines as it could take too much space in stack if number of lines is too many
    int tmpFD = open("tailTemp", O_CREATE | O_RDWR);
    while((n = read(fd, buf, sizeof(buf))) > 0) {
        for(i=0;i<=n;i++) {
            // counting total number of lines in the document to determine from which position to print
            if(buf[i] == '\n') {
                totalLines++;
            }
        }
        // write contents of buffer to temporary file
        write(tmpFD, buf, n);   
    }
    close(tmpFD);
    
    int linesElapsed = 0; // iterator for number of lines enountered in the temp file
    int startPos = totalLines - linesToRead; // lines number in temp file from where to print all lines
    char lastChar = '\n';
    
    // open the temporary file in read mode to print last n lines
    tmpFD = open ("tailTemp", 0); 
    while((n = read(tmpFD, buf, sizeof(buf))) > 0 ) {
        for(i =0;i<n;i++) {
            // print every character once we have reached starting position = (total lines - number of lines to print)
            // or if total lines in file is less than number of lines to print
            if ((linesElapsed >= startPos) || (totalLines < linesToRead)) {
                // ignore empty lines
                if (lastChar == '\n' && buf[i] == '\n')
                    continue;
                lastChar = buf[i];
                printf(1,"%c",buf[i]);
            } else if(buf[i] == '\n') {
                linesElapsed++;
            }
        }
    }
    close(tmpFD);
    unlink("tailTemp");  
}

int main(int argc, char *argv[]) {
    int fd = 0, i, linesToRead = 10;

    for (i = 1; i < argc; i++) {
        if (argv[i][0]=='-') {
            linesToRead = atoi(++argv[i]);   
        } else if ((fd = open(argv[i], 0)) < 0) {
            printf(1, "tail: cannot open %s\n", argv[i]);
            exit();
        }
    }
    if (fd <= 1) {
        tail(0, linesToRead);
    } else {
        tail(fd, linesToRead);
        close(fd);
    }
    exit();
}
