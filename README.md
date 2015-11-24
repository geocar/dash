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

    http=:8080 kdb=127.0.0.1:1234 ./d.darwin

The HTTP response is a blank gif if `?f=gif` is provided in the query string,
and an HTTP 204 for `?f=204`.

KDB must implement a function `dash` which works like `.z.ph`, except note: you must include:

    Connection: keep-alive

in the response when it is appropriate to do so.

#Performance

On a mid-2012 Macbook Air, for async messages, I get about 51k requests per second on localhost:

    Geos-Air:~ geocar$ wrk -t2 -c90 -d3s 'http://127.0.0.1:8080/?f=204&k=hi&v=1'
    Running 3s test @ http://127.0.0.1:8080/?f=204&k=hi&v=1
      2 threads and 90 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     4.61ms   15.97ms 140.72ms   96.15%
        Req/Sec    26.44k     6.12k   47.58k    86.21%
      154692 requests in 3.01s, 9.15MB read
    Requests/sec:  51310.63
    Transfer/sec:      3.03MB

and for sync-messages (.z.ph replacement) I get around 6k requests per second:

    Geos-Air:~ geocar$ wrk -t2 -c90 -d9s 'http://127.0.0.1:8080/?k=hi&v=1'
    Running 9s test @ http://127.0.0.1:8080/?k=hi&v=1
      2 threads and 90 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency    11.11ms   11.50ms 112.05ms   87.14%
        Req/Sec     3.40k   484.83     4.61k    88.89%
      61050 requests in 9.07s, 5.36MB read
      Socket errors: connect 0, read 61042, write 0, timeout 0
    Requests/sec:   6728.89
    Transfer/sec:    604.55KB

using the following:

    dash:{L::x;"HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Type: text/html\r\nContent-Length: 5\r\n\r\n<b>ok"}

In comparison, nodeJS gets 10k queries per second on my laptop:

    require("http").createServer(function(req,res){
      res.writeHead(200);res.end()
    }).listen(8080)

and KDB's built-in webserver gets 2k queries per second:

    \p 8080
    .z.ph:{.h.hy[`html;"ok"]}

