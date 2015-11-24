[d.c](d.c) is a [very fast](#performance) HTTP-to-KDB proxy.

It supports a synchronous mode which works like a faster `.z.ph` and an asynchronous mode
which sends the HTTP response before pipelining the messages into KDB.

To implement async, [d.c](d.c#L65) recognises `?f=` in the query string to select the
HTTP content: `?f=204` for an HTTP 204, and `?f=gif` for a 32-byte blank gif.

#Building

The makefile assumes kdb/q is installed in `$HOME/q` and that you have
the [C bindings](http://kx.com/q/d/c.htm) installed in `$HOME/q/c`:

    q/c/k.h
    q/c/l64/c.o
    q/c/m64/c.o

#Usage
Port numbers are configured using environment variables:

    http=:8080 kdb=127.0.0.1:1234 ./d

The HTTP response is a blank gif if `?f=gif` is provided in the query string,
and an HTTP 204 for `?f=204`.

KDB must implement a function `dash` which works like `.z.ph`, except note: you must include:

    Connection: keep-alive

#Performance
##Setup

KDB+ is configured as:

    \p 1234
    .z.ph:{.h.hy[`html;"ok"]}
    dash:{L::x;"HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nContent-Length: 5\r\n\r\n<b>ok"}

NodeJS is configured as:

    require("http").createServer(function(req,res){
      res.writeHead(200);res.end()
    }).listen(3000)

##OSX 10.11.1 on Mid-2012 Macbook Air (2GHz i7)
###Async: 51k/sec

    Geos-Air:~ geocar$ wrk -t2 -c90 -d3s 'http://127.0.0.1:8080/?f=204&k=hi&v=1'
    Running 3s test @ http://127.0.0.1:8080/?f=204&k=hi&v=1
      2 threads and 90 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     4.61ms   15.97ms 140.72ms   96.15%
        Req/Sec    26.44k     6.12k   47.58k    86.21%
      154692 requests in 3.01s, 9.15MB read
    Requests/sec:  51310.63
    Transfer/sec:      3.03MB

###Sync (.z.ph replacement): 25k/sec

    Geos-Air:~ geocar$ wrk -t2 -c90 -d3s 'http://127.0.0.1:8080/?k=hi&v=1'
    Running 3s test @ http://127.0.0.1:8080/?k=hi&v=1
      2 threads and 90 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     3.73ms  497.42us  11.81ms   91.29%
        Req/Sec    12.13k   691.11    12.88k    90.32%
      74857 requests in 3.10s, 6.57MB read
    Requests/sec:  24136.76
    Transfer/sec:      2.12MB

###NodeJS: 10k/sec

    Geos-Air:~ geocar$ wrk -t2 -c90 -d9s 'http://127.0.0.1:3000/'
    Running 9s test @ http://127.0.0.1:3000/
      2 threads and 90 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     8.65ms    1.46ms  63.20ms   96.41%
        Req/Sec     5.25k   445.35     5.76k    91.67%
      93997 requests in 9.01s, 10.13MB read
    Requests/sec:  10437.06
    Transfer/sec:      1.12MB

###KDB: 2k/sec

The motivation for [d.c](d.c).

    Geos-Air:~ geocar$ wrk -t2 -c90 -d9s 'http://127.0.0.1:1234/'
    Running 9s test @ http://127.0.0.1:1234/
      2 threads and 90 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency    23.56ms   94.75ms 790.67ms   96.31%
        Req/Sec     3.28k   774.95     3.83k    93.88%
      16431 requests in 9.10s, 1.36MB read
    Requests/sec:   1804.94
    Transfer/sec:    153.35KB

##Linux 3.18.21 on 3.50GHz XEON

This is a [Cadence Time Series appliance](https://www.scalableinformatics.com/cadence)
running the 8-core model.

###Async: 135k/sec

    $ ./wrk -t2 -c90 -d3s 'http://127.0.0.1:8080/?f=204&k=hi&v=1'
    Running 3s test @ http://127.0.0.1:8080/?f=204&k=hi&v=1
      2 threads and 90 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     1.98ms   14.37ms 227.27ms   98.58%
        Req/Sec    67.71k    10.92k   80.98k    58.33%
      404058 requests in 3.00s, 23.89MB read
    Requests/sec: 134619.41
    Transfer/sec:      7.96MB

###Sync (.z.ph replacement): 62k/sec

    $ ./wrk -t2 -c90 -d3s 'http://127.0.0.1:8080/?k=hi&v=1'
    Running 3s test @ http://127.0.0.1:8080/?k=hi&v=1
      2 threads and 90 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     2.92ms   14.03ms 199.54ms   98.60%
        Req/Sec    31.51k     1.49k   33.87k    93.33%
      188080 requests in 3.00s, 16.50MB read
    Requests/sec:  62663.59
    Transfer/sec:      5.50MB


###NodeJS: 21k/sec

    $ ./wrk -t2 -c90 -d3s 'http://127.0.0.1:3000/?k=hi&v=1'
    Running 3s test @ http://127.0.0.1:3000/?k=hi&v=1
      2 threads and 90 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     4.30ms    1.67ms  36.76ms   97.84%
        Req/Sec    10.77k     1.31k   11.49k    93.33%
      64309 requests in 3.00s, 6.93MB read
    Requests/sec:  21424.52
    Transfer/sec:      2.31MB

###KDB: 27k/sec

    $ ./wrk -t2 -c90 -d3s 'http://127.0.0.1:1234/'
    Running 3s test @ http://127.0.0.1:1234/
      2 threads and 90 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     3.17ms  312.83us   4.31ms   75.94%
        Req/Sec    13.63k   590.50    15.90k    86.67%
      81375 requests in 3.00s, 6.52MB read
    Requests/sec:  27106.72
    Transfer/sec:      2.17MB

