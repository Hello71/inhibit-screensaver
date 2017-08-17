#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    int pipefd[2];
    if (pipe(pipefd)) {
        perror("pipe");
        exit(1);
    }

    if (fork()) {
        close(pipefd[0]);
        fcntl(pipefd[1], FD_CLOEXEC);

        // do work
        puts("Working...");
        sleep(10);

        // works even if process dies!
        kill(getpid(), SIGKILL);
    } else {
        close(pipefd[1]);
        close(0);
        dup2(pipefd[0], 0);
        execl("./inhibit-screensaver", "./inhibit-screensaver", "c-inhibitor-example", "testing", (char *)NULL);
        exit(1);
    }
}
