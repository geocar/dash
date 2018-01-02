#ifndef __APPLE__
#define _GNU_SOURCE
#define accept_noblock(s) accept4(s,0,0,SOCK_NONBLOCK)
#else
#define accept_noblock(s) (f=accept(s,0,0),fcntl(f,F_SETFL,O_NONBLOCK),f)
#endif

#include "k.h"
#include <stdlib.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <string.h>
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
#include <sys/resource.h>
#include <sched.h>
#endif

static pthread_mutex_t tm=PTHREAD_MUTEX_INITIALIZER;static pthread_cond_t tc=PTHREAD_COND_INITIALIZER;
ZK td;ZS kdbhost;ZI kdbport;

#define NM 1024

#if __linux__
#define TCP_NOPUSH TCP_CORK
#include <sys/epoll.h>
#define QSZ sizeof(struct epoll_event)
ZI Qq(V){R epoll_create1(0);}ZV Qa(I k,I f){struct epoll_event ev={0};ev.data.fd=f;ev.events=EPOLLIN|EPOLLRDHUP|EPOLLERR|EPOLLET;epoll_ctl(k,EPOLL_CTL_ADD,f,&ev);}ZI Qw(I k,V*x,I t){R epoll_wait(k,x,NM,t);}ZI Qf(I k,V*x){struct epoll_event*e=x;I f=e->data.fd;if(e->events&(EPOLLRDHUP|EPOLLHUP)){epoll_ctl(k,EPOLL_CTL_DEL,f,e);close(f);f=-1;}R f;}
#else
#include <sys/event.h>
#define QSZ sizeof(struct kevent)
I Qq(V){R kqueue();}ZV Qa(I k,I f){struct kevent ev;EV_SET(&ev,f,EVFILT_READ,EV_ADD|EV_CLEAR,0,NM,NULL);kevent(k,&ev,1,0,0,0);}ZI Qw(I k,V*x,I t){struct timespec tv={0};tv.tv_sec=5;R kevent(k,0,0,x,NM,t>0?&tv:0);}ZI Qf(I k,V*x){struct kevent*e=x,ev;I f;if(e->flags&EV_EOF){EV_SET(&ev,e->ident,EVFILT_READ,EV_DELETE,0,0,0);close(e->ident);kevent(k,&ev,1,0,0,0);f=-1;}else{/*signed*/f=e->ident;}R f;}
#endif

ZV sc(I f,I b,I*g){if(*g!=b)*g=b,setsockopt(f,IPPROTO_TCP,TCP_NOPUSH,&b,sizeof(b));} ZV sa(I n){
#ifdef __APPLE__
extern int thread_policy_set(thread_t thread, thread_policy_flavor_t flavor, thread_policy_t policy_info, mach_msg_type_number_t count);
{thread_extended_policy_data_t ep;ep.timeshare=FALSE;thread_policy_set(mach_thread_self(),THREAD_EXTENDED_POLICY,(thread_policy_t)&ep,THREAD_EXTENDED_POLICY_COUNT);
};{thread_affinity_policy_data_t ap;ap.affinity_tag=n+1;thread_policy_set(mach_thread_self(),THREAD_AFFINITY_POLICY,(thread_policy_t)&ap,THREAD_AFFINITY_POLICY_COUNT);};
#else
{cpu_set_t c;CPU_ZERO(&c);CPU_SET(n,&c);pthread_setaffinity_np(pthread_self(),sizeof(c),&c);}
#endif
}

ZV writer(I f,S b,I n){if(write(f,b,n)!=n)close(f);}
#define cwrite(f,b) writer(f,b,sizeof(b)-1)
ZV poop(I f){cwrite(f,"HTTP/1.1 500 problem\r\nConnection:close\r\nContent-Length: 9\r\nContent-Type: text/html; charset=us-ascii\r\n\r\n<h1>oop\r\n");close(f);}
#define BLANK "Content-Type: image/gif\r\nContent-Length: 32\r\n\r\nGIF89a\1\0\1\0\0\0\0!\371\4\1\0\0\0\0,\0\0\0\0\1\0\1\0\0\2"
#define OK(N,K) "HTTP/1.1 "N" ok\r\nConnection: " K "\r\n"
#define END204 "Content-Length: 0\r\n\r\n"
ZI http(I d,I f,C*p,I r,I*sd){I i,o=0,b=0,s=0,w=0,g=0,e=0,c=0,cf=1,m=0;K q=0,a=ktn(11,0),v=ktn(0,0);
  for(i=b=0;i<r;++i){switch(p[i]){
  case'\n':if(!e)e=i;if(!q){q=kpn(p+w,g-w);if('\r'==(cf=p[i-1]))cf=p[i-2];}else if(!c){w=!!strchr("kK1",cf);if(!o)sc(d,0,sd);else if(w){if('g'==o)cwrite(f,OK("200","keep-alive")BLANK);else cwrite(f,OK("204","keep-alive")END204);}else{if('g'==o)cwrite(f,OK("200","close")BLANK);else cwrite(f,OK("204","close")END204);close(f);}a=k(o?-d:d,"dash",knk(2,q,xD(a,v),0),0,0);b=i+1;if(!o){if(!a){poop(f);R 0;}if(10!=a->t){r0(a);poop(f);R 0;}writer(f,kC(a),a->n);r0(a);if(!w)close(f);}if(b==r)R 1;q=0;o=0;a=ktn(11,0),v=ktn(0,0);}else{if((c-m)==10&& !strncasecmp(p+m,"connection",c-m))cf=p[s];js(&a,sn(p+m,c-m));jk(&v,kpn(p+s,e-s));}w=e=g=s=c=0;m=i+1;break;
  case' ':case'\t':case'\r':if(w&&!g)g=i;if(s==(e=i))++s;break;
  case':':if(!c)s=1+(c=i);break;
  case '/':if(!w)w=i+1;case '?':case '&':if(r>=(i+4)&&p[i+1]=='f'&&p[i+2]=='=')o=p[i+3];default: e=0;break;}}if(a)r0(a),r0(v);if(q)r0(q);R b;}
ZV*run(I n){I*s=kI(td)+n;C q[QSZ*NM],b[8192];I h,r,f,k,d,c,sd=69; sa(n+1);d=khpu("127.0.0.1",kdbport,"dash");
  k=1;setsockopt(d,IPPROTO_TCP,TCP_NODELAY,&k,sizeof(k));
  k=1048576;if(-1==setsockopt(d,SOL_SOCKET,SO_RCVBUF,&k,sizeof(d)))oops("SNDBUF"); sc(d,1,&sd);
  k=1048576;if(-1==setsockopt(d,SOL_SOCKET,SO_SNDBUF,&k,sizeof(d)))oops("SNDBUF"); sc(d,1,&sd);
  pthread_mutex_lock(&tm);k=*s=Qq();pthread_mutex_unlock(&tm);pthread_cond_signal(&tc);//async
  for(c=-1;;){DO(h=Qw(k,q,c),if(f=Qf(k,q+i*QSZ))if((r=read(f,b,sizeof(b)))==sizeof(b))poop(f);else if(r>0)if(http(d,f,b,r,&sd))c=1);if(h<=0){sc(d,0,&sd);sc(d,1,&sd);c=-1;}}
}
ZV loop(I s,I t){struct timeval tv={0};I f,r=0;tv.tv_sec=5;for(;;)if(-1!=(f=accept_noblock(s))){setsockopt(f,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));Qa(kI(td)[r],f);++r;r=r%t;}}
ZI busy(I t){DO(t,if(((volatile)kI(td)[t])==-1)R 1);R 0;}
ZS var(S x,S d){R (x=getenv(x))?x:d;}ZS hp(S x,I*p,I d){S c=strchr(x=strdup(x),':');*p=d;P(!c,x);*p=atoi(c+1);*c=0;R x;}
int main(int argc,char *argv[]){
  S host;pthread_t id;struct linger lf={0};struct sockaddr_in sin={0};struct rlimit r;I port,o,t,n=EXTRA+WANT,s=-1;
  getrlimit(RLIMIT_NOFILE, &r);if(r.rlim_cur<n&&(r.rlim_max==RLIM_INFINITY||r.rlim_max>n)){r.rlim_cur=n;P(setrlimit(RLIMIT_NOFILE,&r)==-1,oops("setrlimit"));}
  if(!*(kdbhost=hp(var("kdb",""),&kdbport,1234)))kdbhost="127.0.0.1";if(kdbport<=0)oops("$kdb");
  pthread_attr_t a;P(pthread_attr_init(&a)==-1||pthread_attr_setscope(&a,PTHREAD_SCOPE_SYSTEM)==-1||pthread_attr_setdetachstate(&a,PTHREAD_CREATE_DETACHED)==-1,oops("pthread_attr_init"));
  o=1;setsockopt(s=socket(sin.sin_family=AF_INET,SOCK_STREAM,IPPROTO_TCP),SOL_SOCKET,SO_REUSEADDR,&o,sizeof(o));lf.l_onoff=1;lf.l_linger=0;setsockopt(s,SOL_SOCKET,SO_LINGER,&lf,sizeof(lf));
#ifdef __linux__
  o=5;setsockopt(s,SOL_TCP,TCP_DEFER_ACCEPT,&o,sizeof(o));
#endif
  if(!*(host=hp(var("http",""),&port,8080)))host="0";if(port<=0||inet_aton(host,&sin.sin_addr)==-1)oops("$http");sin.sin_port=htons(port);if(0>bind(s,(struct sockaddr*)&sin,sizeof(sin)))oops("bind");
  t=sysconf(_SC_NPROCESSORS_ONLN);if(t<2)t=2;if(0>listen(s,t*n))oops("listen");--t;
  td=ktn(KI,t);DO(t,kI(td)[i]=-1);DO(t,if(pthread_create(&id,&a,(V*)run,(V*)i)==-1)oops("pthread_create"));
  sa(0);while(busy(t)){pthread_mutex_lock(&tm);pthread_cond_wait(&tc,&tm);pthread_mutex_unlock(&tm);}
  loop(s,t);R 0;}
