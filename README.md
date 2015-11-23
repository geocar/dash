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

On a mid-2012 Macbook Air I get about 51k requests per second on localhost:

    Geos-Air:~ geocar$ wrk -t2 -c90 -d3s 'http://127.0.0.1:8080/?f=204&k=hi&v=1'
    Running 3s test @ http://127.0.0.1:8080/?f=204&k=hi&v=1
      2 threads and 90 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     4.61ms   15.97ms 140.72ms   96.15%
        Req/Sec    26.44k     6.12k   47.58k    86.21%
      154692 requests in 3.01s, 9.15MB read
    Requests/sec:  51310.63
    Transfer/sec:      3.03MB

In comparison, nodeJS gets 10k queries per second:

    require("http").createServer(function(req,res){
      res.writeHead(200);res.end()
    }).listen(8080)

and KDB's built-in webserver gets 2k queries per second:

    \p 8080
    .z.ph:{.h.hy[`html;"ok"]}

#Configuring

Port numbers are configured using environment variables:

    http=8080 kdb=127.0.0.1:1234 ./d.darwin

The HTTP response is a blank gif if `?f=gif` is provided in the query string,
and an HTTP 204 otherwise.

