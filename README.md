# xbayOS
a tiny operating system kernel (developing)

一个简易的操作系统内核，在bochs中运行。

## TODO
- [x] MBR -> loader
- [x] paging
- [x] loader -> kernel
- [x] interrupt handler
- [x] debug tools
- [ ] memory management system
- [ ] kernel level thread
- [ ] locks
- [ ] IO
- [ ] user process
- [ ] hard disk driver
- [ ] file system
- [ ] ...

## how to build

1. install and configure bochs
   
You need install [bochs](http://bochs.sourceforge.net/getcurrent.html) first, and create a hard disk image like this:`ata0-master: type=disk, path="the path", mode=flat`.


It's better to put bochs in this direcotry(`xbayOS`).

2. make
```
make all
```

3. run
```
your_bochs_path/bochs -f your_bochs_configure_file_path
```