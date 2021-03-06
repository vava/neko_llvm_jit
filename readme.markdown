What it is?
===========

It is a [neko vm](http://nekovm.org) with LLVM based JIT.

Doesn't neko vm already have a JIT?
-----------------------------------

Yes, it has. But it works only on x86 architecture and most of modern computers run 64bit OSes.
But also we think LLVM has so much optimizations built in that JIT based on it can be much faster.

Any proof?
----------

Well, so far on real working code it gives as much speed up as neko own JIT does. It does take a lot of time to JIT everything but we're working on it.

What do I need to compile it?
=============================

First of all, it can be done only on Linux at this stage.
Using LLVM on Windows is more pain than we would want to experience right now.

Prerequisites:

1. gcc/g++ compiler.
2. LLVM, 2.8.
   It can be downloaded from [llvm site](http://llvm.org/) or
   on Debian based systems (like Ubuntu 10.10) with  
   `$ sudo aptitude install llvm-dev`  
   on Fedora you will have to download and build LLVM yourself as the one from repository is too old

3. ruby/rake, again, on Debian/Ubuntu  
   `$ sudo aptitude install rake`  
   on Fedora (and similar) with  
   `# yum install rubygem-rake`  

4. libgc
5. git, to get sources

To sum up, on Debian based systems you should run

    $ sudo aptitude install gcc llvm-dev rake libgc-dev git

on Fedora and similar

    # yum install gcc rubygem-rake libgc-devel git

and build LLVM from sources.

That's it. Just do

    $ git clone git://github.com/vava/neko_llvm_jit.git
    $ cd neko_llvm_jit
    $ rake

It'll compile neko and run tests.

If you want to use neko to run something else, you should run

    $ LD_LIBRARY_PATH=bin:\`llvm-config --libdir\`:${LD_LIBRARY_PATH} NEKOPATH=boot:bin bin/neko --llvm-jit --llvm-optimizations tests/code/hello_world.n  
    or using power of rake  
    $ rake 'neko_run[tests/code/hello_world.n]'  

Other important questions.
==========================

Why C++?
--------

Three reasons.

* I like it better.
* LLVM C API is undocumented and lacking behind.
* There's a way to convert from C++ to C (through clang), if you the huge need will arise.

Why rake?
---------

It's easy to use but it probably will have to be replaced by Makefiles as project will mature.

Hairy details.
=============================

Creating new test cases
-----------------------------

It's all very simple. Just write neko or haxe source file, put it in tests/code/ directory and run  
    $ rake

You can run individual tests with a little magic provided by rake. Assuming you have neko_test.neko and haxe_test.hx:  
    $ rake run_neko_test
    $ rake run_haxe_test.hx

    running with optimizations enabled:
    $ rake run_opt_neko_test
    $ rake run_opt_haxe_test.hx

    running under gdb and making stack trace on fault:
    $ rake trace_opt_neko_test
    $ rake trace_opt_haxe_test.hx

Cleaning up
-------------------------------

    $ rake clean

This will clean up absolutely everything

Jitting on the fly
------------------------------

Sometimes (like when testing under mod_neko) you want only some scripts to be jitted. To achieve that you can use JIT on the fly primitive.

Just call `run_llvm_jit` primitive.

    var f = $loader.loadprim(("std" + "@") + "run_llvm_jit",1);
    f($exports.__module);

See tests/code/llvm_jit_on.neko for working code.

Note that function where you call that primitive and all functions up the execution stack from it will still be interpreted, but every function you call after this primitive is run will use JIT.

If by any chance neko own JIT is enabled, LLVM JIT won't happen.

Notes about mod_neko
-----------------------------

To enable LLVM JIT on mod_neko you have two choices, you can either use `run_llvm_jit` primitive or put `ModNeko USE_LLVM 1` somewhere in your apache config. You might want to consider putting `ModNeko USE_LLVM_OPT 1` there as well, otherwise it will be slow. You might also want to cache the result of jitting with `neko.Web.cacheModule`.

Note that mod_neko runs number of different threads and for each thread jitting is done separately. So don't be alarmed if you see jitting happening more than once, it'll stop when every thread will have it's own cached version of jitted code.
