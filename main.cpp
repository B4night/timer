#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include "time_wheel.h"
#include "time_heap.h"
#include <sys/time.h>
#include <signal.h>

int pipefd[2];

void sig_handler(int sig) {
    printf("enter sig_handler\n");
    int saved_errno = errno;
    int msg = sig;
    write(pipefd[1], &msg, 1);
    errno = saved_errno;
}

void epfd_add(int epfd, int fd, int event) {
    struct epoll_event ev;
    ev.events = event;
    ev.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        fprintf(stderr, "epfd_add failure\n");
        exit(1);
    }
}

void func1(void*) {
    printf("ha ha ha\n");
}

void func2(void*) {
    printf("ba ba ba\n");
}

int main() {
    // time_wheel tw;
    time_heap th;

    for (int i = 1; i <= 5; i++) {
        // tw.add_timer(i, func1, NULL);
        // tw.add_timer(i + 5, func2, NULL);
        th.add_timer(i, func1, NULL);
        th.add_timer(i + 5, func2, NULL);
    }

    //setitimer
    struct itimerval tm;
    tm.it_interval.tv_sec = 1;
    tm.it_interval.tv_usec = 0;
    tm.it_value.tv_sec = 1;
    tm.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &tm, NULL); 

    //sigaction
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = sig_handler;
    sigaction(SIGALRM, &act, NULL);

    pipe(pipefd);

    int epfd = epoll_create(128);
    epfd_add(epfd, pipefd[0], EPOLLIN);
    struct epoll_event evs[128];
    while (1) {
        int num = epoll_wait(epfd, evs, 128, -1);
        for (int i = 0; i < num; i++) {
            if (evs[i].data.fd == pipefd[0]) {
                char signal[1024];
                int cnt = read(pipefd[0], signal, 1024);
                for (int k = 0; k < cnt; k++) {
                    switch (signal[k]) {
                        case SIGALRM:
                            // tw.tick();
                            th.tick();
                            break;
                        case SIGTERM:
                            break;
                    }
                }
            }
        }
    }

}