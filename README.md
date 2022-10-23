# High-Efficiency timer

时间轮和时间堆的c++11实现

## 时间轮 time_wheel

实现在`time_wheel.h`中给出

`time_wheel`适用的定时器`wheel_timer`结构如下

``` C++
class wheel_timer {
public:
    wheel_timer(int rot, int slo, void (*func)(void*), void* argument) : 
    rotation(rot), time_slot(slo), cb_func(func), arg(argument) {}
    
    bool operator==(const wheel_timer& mt) {		//用于std::find
        return this->rotation == mt.rotation &&
                this->time_slot == mt.time_slot &&
                this->cb_func == mt.cb_func && 
                this->arg == mt.arg;
    }
public:
    int rotation;			//定义了time_wheel转到第几圈时此定时器道奇
    int time_slot;			//在time_wheel的哪一个slot中
    void (*cb_func)(void*); //call back function
    void* arg;  			//cb_func's argument
};
```



`time_wheel`中使用list来存储一个slot上的定时器, 并用一个list的数组来表示多个槽, 类似于哈希表的链表式.

主进程每隔一段设定的时间, 如1s调用tick()函数, 在tick中对当前指向的slot的list进行遍历, 如果有到期的定时器则执行定时器的任务.

注: wheel_time::rotation的值为0时才执行, 否则只是将其值减一, 下一轮再执行

## 时间堆 time_heap

实现在`time_heap.h`中给出

`time_heap`适用的定时器`heap_timer`结构如下

``` C
class heap_timer {
public:
    heap_timer(int delay, void (*func)(void*), void* argument): cb_func(func), arg(argument)  {
        expire = time(NULL) + delay;	
        //定时器将在delay时间后到期,则绝对时间expire就是当前时间加上delay
    }

public:
    time_t expire;	//绝对时间
    void (*cb_func)(void*);
    void* arg;
};

bool operator< (const heap_timer& h1, const heap_timer& h2) {  //用于std::less<heap_timer>
    return h1.expire > h2.expire;
}
```

`time_heap`中使用小根堆来存储所有的定时器, 到期时间(绝对时间)最小的在堆顶, 每次调用tick()时取出堆顶元素执行即可.



## 主进程中对于SIGALRM的处理

在主进程中对于SIGALRM的处理是比较tricky的

将SIGALRM对应的回调函数设置为`sig_handler`, 但是在`sig_handler`中不对SIGALRM进行处理, 而是往管道的写端写数据. 使用epoll监测管道的读端, 如果有EPOLLIN信号则说明有信号需要被处理.

不在信号处理机制中处理信号, 而是在其他地方处理可以有效避免当在信号处理机制中又来了其他信号从而导致的竟态问题.

主程序框架如下

``` C++
void sig_handler(int sig);


int main() {
    setitimer();	//设置周期性发送给主程序的SIGALRM信号
    sigaction(SIGALRM, ...);	//设置SIGALRM的信号处理机制
    pipe(pipefd);	//创建管道, 信号处理函数往管道写端写数据, epoll监测管道的读端
    
    while (1) {
        int num = epoll_wait();	//
        for (int i = 0; i < num; i++) {
            if (evs[i].data.fd == pipefd[0]) {	//此时说明管道的读端有EPOLLIN事件发生
                char signal[1024];
                int cnt = read(pipefd[0], signal, 1024);	//将管道中的数据读到signal中
                for (int k = 0; k < cnt; k++) {
                	switch (signal[k]) {
                        case SIGALRM:
                            //调用时间轮或时间堆的tick()函数
                        default:
                            //其他信号的处理
                    }   
                }
            }
        }
    }
}
```

 
