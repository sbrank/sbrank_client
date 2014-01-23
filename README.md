Starbow Ranking Client (sbrank_client) Readme

This is the client that enables the user to play ranked games of Starbow.
It does replay parsing and looks up the users profile and uploads the information to the server.

main.cpp is the client sourcecode (this is currently a mess unfortunately)
python/extractreplay.py parses the replay and outputs in a text file
libraries/* are libraries I have included in the project that don't need to be staticly linked to lib files
extractreplay/* is the last built version of extractreplay.py as a standalone (I think its easier to add this to version control even though its a binary)


sbrank_client has been written in C++ and compiled with Microsoft Visual Studio 2010 on Windows 8.1 x64 but it should run on 
all versions of windows that SC2 supports (ie XP/Vista/7/8.x).

libcurl: http://curl.haxx.se/libcurl/ is required to compile the project (http://curl.haxx.se/download/libcurl-7.18.0-win32-msvc.zip)
boost: http://www.boost.org/ may probably be required in the future but current it is not required

The code has mostly been written in such a way that should be fairly easy to compile with gcc and as such be able to support MacOSX,
cxfreeze also supports MacOSX so this should not be a problem either. I will fix the code to compile with gcc on linux at some point later.
