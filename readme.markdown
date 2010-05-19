What it is?
===========

It is a [neko vm](http://nekovm.org) with LLVM based JIT.

Doesn't neko vm already have a JIT?
-----------------------------------

Yes, it has. But it works only on x86 architecture and most of modern computers run 64bit OSes.
But also we think LLVM has so much optimizations built in that JIT based on it can be much faster.

Any proof?
----------

Not yet, we haven't got to that stage yet. Stay tuned though.

What do I need to compile it?
=============================

First of all, it can be done only on Linux at this stage.
Using LLVM on Windows is more pain than we would want to experience right now.

Prerequisites:

1. gcc/g++ compiler.
2. LLVM, obviously.
   It can be downloaded from [llvm site](http://llvm.org/) or
   on Debian based systems (like Ubuntu) with
   `$ sudo aptitude install llvm-dev`

3. ruby/rake, again, on Debian/Ubuntu
   `$ sudo aptitude install rake`

4. git, to get sources

That's it. Just do

    $ git clone git://github.com/vava/neko_llvm_jit.git
    $ cd neko_llvm_jit
    $ rake

It'll compile neko and run tests.

Other important questions.
==========================

Why C++?
--------

Two reasons.

* I like it better.
* LLVM C API is undocumented and lacking behind.
* There's a way to convert from C++ to C (through clang), if you the huge need will arise.

Why rake?
---------

It's easy to use but it probably will have to be replaced by Makefiles as project will mature.
