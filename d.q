\p 1234
buffersize:00:00:30
archive:$[max`archive=key`:.;get`:archive;()!()]

dash:{record first x;"HTTP/1.1 302 ok\r\nLocation: ",,[first ":"vs x[1;`Host];string system"p"],"\r\nContent-Length: 0\r\nConnection: keep-alive\r\n\r\n"}

buffer:([]s:`long$();t:`timestamp$();k:`symbol$();v:`float$());
record:{x:(!/)"S=&"0:@[;1]"?"vs x;buffer,:`t`s`k`v!(.z.p,$[0=count s:x`s;rand 0j;"J"$s],"SF"$x`k`v)}

retain:flip`r`p`z!"nns"$\:()
`retain insert "nns"$(00:00:10.000;  2D; `utc)
`retain insert "nns"$(00:01:00.000; 14D; `utc)
`retain insert "nns"$(01:00:00.000; 60D; `utc)
`retain insert "nns"$(1D;            0W; `utc)
`retain insert "nns"$(1D;            0W; `est)
`retain insert "nns"$(1D;            0W; `pst)

utc:{x};est:{`TZ setenv "US/Eastern";ltime x};pst:{`TZ setenv "US/Pacific";ltime x}

akey:{`$"@"sv string x`z`r}
timebd:{`t`k!((xbar;x`r;(value x`z;`t));`k)};
rollup:`n`v`v2!((count;1);(sum;`v);(sum;(*;`v;`v)));aggr:`n`v`v2!((sum;`n);(sum;`v);(sum;`v2));session:enlist (=;`i;(fby;(enlist;last;`i);`s))
base:([t:`timestamp$();k:`symbol$()] n:`long$(); v:`float$(); v2:`float$())

prune1:{archive[x]:![archive@x;enlist (<;`t;.z.p-y);0b;`symbol$()]}
prune:{a:akey each b:select from retain where p<>0W;a prune1'b.p}

combine:{k:akey z;a:archive[k],:base;b:?[x;y 0;timebd[z];y 1];p:exec a:min t,b:max t from b;archive[k]-:select from a where t within (value p);archive[k]+:b;}
batch2:{a:a@b:first key a:exec max r by z from retain where r<buffersize;c:archive[akey`z`r!(b;a)];combine[c;(();aggr)]each select distinct z,r from retain where r>=buffersize;}
batch:{combine[buffer;(session;rollup)] each select distinct z,r from retain where r < buffersize;batch2`;delete from `buffer where t<(exec max t-buffersize from(first archive));prune`;save`:archive}
.z.ts:{batch`}
\t 10000

pt:{$[x~"now";.z.p;not null t:"J"$x;(1000000000*t)+"p"$1970.01.01;.z.p+(`min`h`d`m`y!(00:01;01:00;01D;31D;365D))[`$n _ x]*"J"$(n:min x?.Q.a)#x]}
dp:{"J"$$[not 0=count x`width;x`width;not 0=count x`maxDataPoints;x`maxDataPoints;"1000"]}

query:{n:dp x;w:pt each x`from`until;s:"n"$%[r:abs(-/)w;2*n];k:akey first select[<r] z,r from(retain where retain.r<=s)uj retain where r<>0Nn and p>=(.z.p-w 0);a:archive[k];?[a;((within;`t;w);(any;((/:;like);`k;(`x;enlist `target))));`t`k!(($["p"];(-;w 1;(xbar;(floor r%n);(-;w 1;`t))));`k);aggr]};
x1:{,[-1_ x;"/>"]}

format:()!()
format[`json]:{.j.j ()xkey select datapoints:(v,'"j"$(t-"p"$1970.01.01)div 1000000000) by target:k from (query x)}
format[`html]:{raze read0 `:d.html}
format[`svg]:{q:query[x];c:c!#[n:count c:exec distinct k from q;","vs x`colorList];w:pt each x`from`until;h:"J"$x`height;p:dp x;m:10 xexp ceiling 10 xlog exec max v from q;r:abs(-/)w;d:exec "j"$((p-p*("j"$(w 1)-t)%r),'(h*v%m)) by k from q;k:key c;,[.h.hta[`svg;`version`xmlns`viewBox!("1.2";"http://www.w3.org/2000/svg";" "sv string 0 0,p,h)],x1[.h.hta[`rect;string `style`x`y`width`height!(`$"fill:#",(x`bgcolor),";";0;0;p;h)]],raze x1 each .h.hta[`polyline;]each  ([]id:string k;fill:n#enlist"none"; stroke:"#",'c@k; points: " "sv'","sv'' string d@k);"</svg>"]}

.z.ph:{x:(`target _(!/)x),(enlist`target)!enlist x[1]@where`target=first x:"S=&"0:last"?"vs x@0;.h.hy[f;format[f:`$x`format;PX::x]]}
