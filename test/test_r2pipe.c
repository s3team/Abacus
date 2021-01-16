
#include <stdio.h>
#include <r_socket.h>



int main() {
    // R2Pipe *r2 = r2pipe_open ("/bin/ls");
    char *res;
    R2Pipe *r2 = r2pipe_open ("radare2 -q0 /bin/ls");
    if (r2) {
        res = r2pipe_cmd (r2, "?e Hello World");
        printf("result: %s\n", res);
        res = r2pipe_cmd (r2, "x");
        printf("result: %s\n", res);
        res = r2pipe_cmd (r2, "?e Hello World");
        printf("result: %s\n", res);
        res = r2pipe_cmd (r2, "pd 20");
        printf("result: %s\n", res);
        r2pipe_close (r2);
        return 0;
    }
    return 1;
}