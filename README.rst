drawPolygonContext
===================
A simple context based off Chad Vernon's (http://www.chadvernon.com/blog/resources/maya-api-programming/) drawCurveContext tool. This just draws polygons instead of curves, which is similar to the create polygon tool.

Features
=========
* Draws a polygon with the input from your mouse/tablet.

Planned Features
================
* More functionality.
* More builds.

Usage
======
Load the plugin using the plugin manager.

Then type this in the mel script editor with a polygon selected:
    setToolTo drawPolygonContext1;

Start drawing!

Compiling with CMake
=====================

**MAKE SURE YOU'RE IN THE BUILD DIRECTORY. Here's how to compile for your os.**

The CMakeLists.txt in the main directory is the cmake to run. Here's some examples.

Linux:

	* cmake -G "Unix Makefiles" ../../../
	* make

Linux without default install:

    * cmake -G "Unix Makefiles" -DMAYA_INSTALL_BASE_PATH=YourMayaPath ../../../
    * make

Linux 2014 and custom install path:

    * cmake -G "Unix Makefiles" -DMAYA_INSTALL_BASE_PATH=YourMayaPath -DMAYA_VERSION=2014 ../../../
    * make

Windows 2014(x64):

    * cmake -G "Visual Studio 10 Win64" -DMAYA_VERSION=2014 ../../../
    * cmake --build . --config Release --target install

Windows 2015(x64):

    * cmake -G "Visual Studio 11 Win64"  ../../../
    * cmake --build . --config Release --target install

Compiling Requirements
======================
* CMake Version 2.8 or greater (http://www.cmake.org/).
* Autodesk Maya 2015 or greater (www.autodesk.com/Maya‎).
