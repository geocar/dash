This is not a rigourous benchmark; I did not use the TechEmpower framework for building my test, nor did I build a version
of dash for TechEmpower:

Comparing a 100 line C program to hundreds or thousands of lines of go or C or Java
is a bit pointless. If the 100 lines of C doesn't do what you want, you'll throw
it away and write a different 100 lines. That's the point of small programs.

The only reason I chose fasthttp instead of ulib because I could not
get ulib to build on my machine.

I built the fasthttp test program by by checking out https://github.com/TechEmpower/FrameworkBenchmarks.git,
and disabling the mysql connection, then running frameworks/Go/fasthttp's `./server-mysql`

Here's fasthttp running on my machine (best of three):

    $ wrk -t2 -c90 -d9s http://localhost:8080/plaintext
    Running 9s test @ http://localhost:8080/plaintext
      2 threads and 90 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency   831.78us  364.48us   7.56ms   70.19%
        Req/Sec    40.55k     3.31k   48.04k    74.44%
      726417 requests in 9.01s, 87.98MB read
    Requests/sec:  80603.64
    Transfer/sec:      9.76MB

and here's dash running with the kdb networking disabled (best of three):

    $ wrk -t2 -c90 -d3s 'http://127.0.0.1:8080/?f=204&k=hi&v=1'
    Running 3s test @ http://127.0.0.1:8080/?f=204&k=hi&v=1
      2 threads and 90 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency   787.72us  213.62us   3.49ms   71.85%
        Req/Sec    44.82k     3.04k   60.44k    83.61%
      271946 requests in 3.10s, 16.08MB read
    Requests/sec:  87671.23
    Transfer/sec:      5.18MB
