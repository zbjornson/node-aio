This demonstrates using aio_read(3) (posix) for an implementation of `fs.readFile()`.

The libuv docs cite [this article](https://blog.libtorrent.org/2012/10/asynchronous-disk-io/#comments) as the reasoning behind using `read()` in a loop instead of using async file operations. The major issue that stands out is that notifications can only be issued using a signal or in a new thread. As a workaround, this demo polls the status of an `aio_read()` request in the event loop. This prevents the majority of the blocking of the event loop because `aio_error(3)` is fast.

Note that this demo still uses the threadpool for `fopen` and `fstat` to avoid blocking as much as possible. (nit: `aio_read()` could be issued in the event loop to avoid that small block.)

Benchmark (time to read a 16 MB file 50 times):  
Node.js v10.x's chunked `fs.readFile()`: 4714 ms  
Approximation of Node.js v8.x's one-shot `fs.readFile()`: 366 ms  
This module: 489 ms
