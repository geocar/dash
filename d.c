#include "k.h"

#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#define WANT 2048 
#define EXTRA 32
ZI oops(S x){perror(x);exit(1);}
#ifdef __APPLE__
#include <mach/mach_init.h>
#include <mach/thread_policy.h>
#include <mach/mach_traps.h>
#else
#include <pthread_np.h>
#endif

static pthread_mutex_t tm=PTHREAD_MUTEX_INITIALIZER;static pthread_cond_t tc=PTHREAD_COND_INITIALIZER;
ZK td;

#define NM 1024

#if __linux__
#include <sys/epoll.h>
#include <netinet/tcp.h>
#define QSZ sizeof(struct epoll_event)
ZI Qq(V){R epoll_create1(0);}ZV Qa(I k,I f){struct epoll_event ev={0};ev.data.fd=f;ev.events=EPOLLIN|EPOLLRDHUP|EPOLLERR|EPOLLET;epoll_ctl(k,EPOLL_CTL_ADD,f,&ev);}ZI Qw(I k,V*x,I t){R epoll_wait(k,x,NM,t);}ZI Qf(I k,V*x){struct epoll_event*e=x;I f=e->data.fd;if(e->events&(EPOLLRDHUP|EPOLLHUP)){epoll_ctl(k,EPOLL_CTL_DEL,f,e);close(f);f=-1;}R f;}
#else
#include <sys/event.h>
#define QSZ sizeof(struct kevent)
I Qq(V){R kqueue();}ZV Qa(I k,I f){struct kevent ev;EV_SET(&ev,f,EVFILT_READ,EV_ADD|EV_CLEAR,0,NM,NULL);kevent(k,&ev,1,0,0,0);}ZI Qw(I k,V*x,I t){struct timespec tv={0};tv.tv_sec=1;R kevent(k,0,0,x,NM,t>0?&tv:0);}ZI Qf(I k,V*x){struct kevent*e=x,ev;I f;if(e->flags&EV_EOF){EV_SET(&ev,e->ident,EVFILT_READ,EV_DELETE,0,0,0);close(e->ident);kevent(k,&ev,1,0,0,0);f=-1;}else{/*signed*/f=e->ident;}R f;}
#endif
#define cwrite(f,b) if(write(f,b,sizeof(b)-1)!=(sizeof(b)-1))close(f)
ZV poop(I f){cwrite(f,"HTTP/1.1 500 problem\r\nConnection:close\r\nContent-Length: 9\r\nContent-Type: text/html; charset=us-ascii\r\n\r\n<h1>oop\r\n");close(f);}
#define BLANK "Content-Type: image/gif\r\nContent-Length: 32\r\n\r\nGIF89a\1\0\1\0\0\0\0!\371\4\1\0\0\0\0,\0\0\0\0\1\0\1\0\0\2"
#define OK(N,K) "HTTP/1.1 "N" ok\r\nConnection: " K "\r\n"
ZV kablank(I f){cwrite(f,OK("200","keep-alive")BLANK);}ZV ccblank(I f){cwrite(f,OK("200","close")BLANK);close(f);}ZV ka204(I f){cwrite(f,OK("204","keep-alive")"\r\n");}ZV cc204(I f){cwrite(f,OK("204","close")"\r\n");close(f);}
ZV http(I d,I f,C*p,I r){I z=0,c='-',b=0,i,nl=0;
  for(i=0;i<r;++i)if(p[i]=='\r')1;else if(p[i]=='\n'){if(!nl&&i>9&&(c=p[i-1])=='\r')c=p[i-2];++nl;if(!z){
if(c=='k'||c=='K'||c=='1')kablank(f);else ccblank(f);
k(d,"dash",kpn(p+b,i-b),0,0);b=i+1;continue;
}z=0;}else if(z>=0&&(z < 10 && (p[i]=="CONNECTION"[z] || p[i]=="connection"[z])))++z;else if(z==10){z=(p[i]==' '||p[i]==':')?10:-1;c=p[i];}else z=-1;
}
#define BUFSZ 8192
ZV sa(I n){
#ifdef __APPLE__
extern int thread_policy_set(thread_t thread, thread_policy_flavor_t flavor, thread_policy_t policy_info, mach_msg_type_number_t count);
{thread_extended_policy_data_t ep;ep.timeshare=FALSE;thread_policy_set(mach_thread_self(),THREAD_EXTENDED_POLICY,(thread_policy_t)&ep,THREAD_EXTENDED_POLICY_COUNT);
};{thread_affinity_policy_data_t ap;ap.affinity_tag=n+1;thread_policy_set(mach_thread_self(),THREAD_EXTENDED_POLICY,(thread_policy_t)&ap,THREAD_EXTENDED_POLICY_COUNT);};
#else
{cpu_set_t c;CPU_ZERO(&c);CPU_SET(n,&c);pthread_setaffinity_np(pthread_self(),sizeof(c),&c);}
#endif
}
ZV*run(I n){I*s=kI(td)+n;C q[QSZ*NM],b[BUFSZ];I r,f,k,d; sa(n+1);d=khpu("127.0.0.1",1234,"dash");
  k=1048576;if(-1==setsockopt(d,SOL_SOCKET,SO_SNDBUF,&k,sizeof(d)))oops("SNDBUF");
  pthread_mutex_lock(&tm);k=*s=Qq();pthread_mutex_unlock(&tm);pthread_cond_signal(&tc);d=-d;//async

for(;;)DO(Qw(k,q,-1),if(f=Qf(k,q+i*QSZ))if((r=read(f,b,sizeof(b)))==sizeof(b)||r<0)poop(f);else if(r>0)http(d,f,b,r))}
ZV loop(I s,I t){struct timeval tv={0};I f,r=0;tv.tv_sec=1;for(;;)if(-1!=(f=accept(s,0,0))){setsockopt(f,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));fcntl(f,F_SETFL,O_NONBLOCK);Qa(kI(td)[r],f);++r;r=r%t;}}
ZI busy(I t){DO(t,if(((volatile)kI(td)[t])==-1)R 1);R 0;}
int main(int argc,char *argv[]){
  pthread_t id;struct linger lf={0};struct sockaddr_in sin={0};struct rlimit r;I o,t,n=EXTRA+WANT,s=-1;
  getrlimit(RLIMIT_NOFILE, &r);if(r.rlim_cur<n&&(r.rlim_max==RLIM_INFINITY||r.rlim_max>n)){r.rlim_cur=n;P(setrlimit(RLIMIT_NOFILE,&r)==-1,oops("setrlimit"));}
  pthread_attr_t a;P(pthread_attr_init(&a)==-1||pthread_attr_setscope(&a,PTHREAD_SCOPE_SYSTEM)==-1||pthread_attr_setdetachstate(&a,PTHREAD_CREATE_DETACHED)==-1,oops("pthread_attr_init"));
  o=1;setsockopt(s=socket(sin.sin_family=AF_INET,SOCK_STREAM,IPPROTO_TCP),SOL_SOCKET,SO_REUSEADDR,&o,sizeof(o));lf.l_onoff=1;lf.l_linger=0;setsockopt(s,SOL_SOCKET,SO_LINGER,&lf,sizeof(lf));
#ifdef __linux__
  o=5;setsockopt(s,SOL_TCP,TCP_DEFER_ACCEPT,&o,sizeof(o));
#endif
  sin.sin_addr.s_addr=INADDR_ANY;sin.sin_port=htons(8080);if(0>bind(s,(struct sockaddr*)&sin,sizeof(sin)))oops("bind");
  t=sysconf(_SC_NPROCESSORS_ONLN);if(t<2)t=2;if(0>listen(s,t*n))oops("listen");--t;
  td=ktn(KI,t);DO(t,kI(td)[i]=-1);DO(t,if(pthread_create(&id,&a,(V*)run,(V*)i)==-1)oops("pthread_create"));
  sa(0);while(busy(t)){pthread_mutex_lock(&tm);pthread_cond_wait(&tc,&tm);pthread_mutex_unlock(&tm);}
  loop(s,t);R 0;}
