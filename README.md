#Building

The makefile assumes kdb/q is installed in `$HOME/q` and that you have
the [C bindings](http://kx.com/q/d/c.htm) installed in `$HOME/q/c`:

    q/c/k.h
    q/c/l64/c.o
    q/c/m64/c.o

#Usage

This is a special-purpose webserver that always returns a specific
static content.

A number of tasks involve collecting data from web browsers which
can basically call a URL, however the URL always responds with
a HTTP 204 or a blank gif and the actual request can be processed
after the client disconnects.

#Performance

This was written on a sunday afternoon, so I imagine there are improvements
that can be made, nevertheless it's already pretty fast.

On a mid-2012 Macbook Air I get about 38k requests per second on localhost:

    Geos-Air:~ geocar$ wrk -t2 -c90 -d9s http://127.0.0.1:8080/
    Running 9s test @ http://127.0.0.1:8080/
      2 threads and 90 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     2.36ms  145.04us   5.16ms   82.00%
        Req/Sec    19.03k   335.33    19.99k    71.11%
      341045 requests in 9.00s, 39.03MB read
    Requests/sec:  37878.01

In comparison, nodeJS gets 10k queries per second:

    require("http").createServer(function(req,res){
      res.writeHead(200);res.end()
    }).listen(8080)

and KDB's built-in webserver gets 2k queries per second:

    \p 8080
    .z.ph:{.h.hy[`html;"ok"]}


#Configuring

Currently hardcoded to [listen on port 8080](d.c#L75) and connect to kdb on [port 1234](d.c#L61), and it serves [32-byte blank gifs](/geocar/dash/blob/master/d.c#L47).
