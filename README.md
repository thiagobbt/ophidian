<img src="https://travis-ci.org/eclufsc/ophidian.svg?branch=travis" >

<img src="https://raw.githubusercontent.com/eclufsc/ophidian/master/logo.png" width=200>
Ophidian
========

Ophidian is a multi-platform library for ease the teaching and research on physical design topics.

[Documentation](http://eclufsc.github.io/ophidian/doc/html/index.html)

Code Your Own Timing-Driven Placement Algorithm With the Brand-New TDP Module
-------------------------
[Sample Code](https://github.com/eclufsc/ophidian/blob/master/apps/tdp/main.cpp)

How to build Ophidian
---------------------
```
cmake [-DCMAKE_BUILD_TYPE=(Release|Debug)] [-DBUILD_GUI:BOOLEAN=true] $OPHIDIAN_ROOT
make [-jX]
```

UDDAC 2016 Demonstration
-------------------------
To build the app presented in University Demonstration at DAC 2016, you need to build de GUI module, passing the argument ```-DBUILD_GUI:BOOLEAN=true``` to the cmake command.
The sample app is located inside the apps/uddac2016 folder.

Eclipse user attention
-------------------------

Follow this steps to set-up your Eclipse Project with Ophidian

```bash
mkdir ../ophidian-build
cd ../ophidian-build
mkdir debug
cd debug
cmake -G"Eclipse CDT4 - Unix Makefiles" -D CMAKE_BUILD_TYPE=Debug ../../ophidian
```
Same for Release (switch "Debug" to "Release")

You must add a preprocessor macro to enable Eclipse to use c++11 features (e.g., like unordered_map, unique_ptr, etc.)

__cplusplus=201103L

Step by Step: Right-Click on Project -> Properties -> C/C++ General -> Preprocessor Include Paths, Macros etc. -> Entries Tab -> Language GNU C++ -> Select CDT User Setting Entries from Setting Entries -> Click Add... -> Type = Preprocessor Macro, Name = __cplusplus, Value = 201103L


