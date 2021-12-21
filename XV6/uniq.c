#include "types.h"
#include "stat.h"
#include "user.h"

char buf[512];
char previous[1024];
char curr[1024];

// Checks case insensitive diff between two strings pre and curr
// Returns: 1 if unequal, 0 if equal
int checkIDiff(char *pre, char *curr) {
    if (strlen(pre)!=strlen(curr)) {
        return 1;
    }
    for (int i=0; i<strlen(pre); ++i)  {
        char p  =  (pre[i] > 64 && pre[i] < 91)? pre[i] + 32:pre[i];
        char c  =  (curr[i] > 64 && curr[i] < 91)? curr[i] + 32:curr[i];
        if (p!=c)  {
            return 1;
        }
    }
    return 0;
}

// Function called only when prev and current are different
// This prints the previous line along with count if required, then copies current into previous
void process(char *flags, int *dup)  {
    if  (strlen(previous)>0) {
        if (strchr(flags, 'c')) {
            printf(1, "%d %s",*dup,previous);
            *dup=0;
        } else if (strchr(flags, 'd')) {
            if (*dup>1) {
               printf(1,"%s", previous);
           }
           *dup =0;
       } else {
        printf(1,"%s" ,previous);
        }
    }
    strcpy(previous, curr);
}

void uniq(int fd, char *flags) {
    int i = 0, n, c, dup, eof = 0;
    char *caseInsensitiveCheck;
    c=0, dup=0;
    caseInsensitiveCheck = strchr(flags, 'i');
    int processed = 0;
    int lastLine = 0;

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        
        //if less than 512 bytes are read it means the file ends in this iteration
        if (n<512) {
            eof = 1;
        }

        for (i = 0; i < n; i++) {
            curr[c] = buf[i];
        processeof:
            if ((lastLine == 1) || buf[i] == '\n' || (i == (n-1) && eof == 1))  {
                processed = 0;
                
                //adding newline to end of current string
                if(lastLine == 1) {
                    curr[c] = '\n';
                } else if((i == (n-1) && eof == 1) && curr[c] != '\n') {
                    c = c + 1;
                    curr[c] = '\n';
                } 

                // Terminate the current string as new line is reached
                curr[c + 1] = '\0';
                c=0;
                
                if (strlen(previous)>0) dup++;
                
                // case insensitive check
                if (caseInsensitiveCheck) {
                    if (checkIDiff(previous,curr)) {
                        process(flags, &dup);
                        processed = 1;
                    }
                } 
                // case sensitive check
                else if (strcmp(previous, curr) != 0) {
                    process(flags,&dup);
                    processed = 1;
                }
                memset(curr, 0, 1024);

                //handling for last line in file
                if((i == (n-1) && eof == 1) || lastLine == 1) {
                    if(processed == 1)
                        dup = 1;
                    else
                        dup++;
                    process(flags, &dup);
                }
            } else {
                c++;
            }
        }
    }

    //handling for last line in file when file size is multiple of 512 ie the buffer size
    if(c != 0) {
        lastLine = 1;
        goto processeof;
    }  
}


int main(int argc,char *argv[]) {
    int fd, i, c;
    char *f = "cdi";
    char flags[4];
    c=0;
    // iterate over all arguments
    for (i = 1; i < argc; i++) {
        if (argv[i][0]=='-'&&strlen(argv[i])==2) {
            if (strchr(f, argv[i][1])) {
                flags[c]=argv[i][1];
                c++;
            } else {
                printf(1, "uniq: invalid flag\n", argv[i]);
                exit(); 
            }
        } else if ((fd = open(argv[i], 0)) < 0) {
            printf(1, "uniq: invalid argument\n", argv[i]);
            exit();
        }
    }

    if (strchr(flags, 'c')&&strchr(flags, 'd')) {
        printf(1, "uniq: flags -c and -d can't execute at the same time\n");
        if (fd>1)  close(fd);
        exit();
    }

    if (fd <= 1) {
        uniq(0,flags);
    } else {
        uniq(fd, flags);
        close(fd); 
    }
    exit();
}