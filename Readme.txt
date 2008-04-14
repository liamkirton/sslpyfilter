================================================================================
SslPyFilter 0.3.2
Copyright ©2008 Liam Kirton <liam@int3.ws>

14th April 2008
http://int3.ws/
================================================================================

Overview:
---------

SslPyFilter is a tool that allows the capture and modification of cleartext
traffic before encryption/after decryption by the Windows SSL SSPI API. This
means that encrypted SSL traffic can be modified prior to being sent/received
by a target process, without the need for a man-in-the-middle proxy.

This is very much a proof-of-concept tool.

0.3.2 adds experimental support for the expansion of outgoing buffers. This
is not currently possible for received data (although, it could probably be
achieved with an even more tremendous hack).

DLL injection and API hooking techniques are used to capture the data, whilst
modification is performed by Python language scripts loaded dynamically by the
SslPyFilter.exe application. Buffer expansion is achieved through the addition
of a layered service provider (borrowed from LspPyFilter) and a cunning idea :)

Please email any comments, questions or suggestions to liam@int3.ws.

Limitations:
------------

Currently, the following limitations apply:

 - The length of a modified incoming data buffer (returned from decrypt_filter)
   must be equal to that of the original.
 - No documentation is provided and only a single script example is included.
 
Installation:
-------------

Requires: Python 2.5.x (http://www.python.org/).

 1. Open a command prompt and navigate to the SslPyFilter directory.
 2. > copy /y SslHook.dll C:\Windows\System32
 3. > regsvr32 SslPyFilterMarshal.dll
 4. > LspInstall.exe /Ifs /Install 1001 SslLsp C:\SslPyFilter\SslLsp.dll
 5. > LspInstall.exe /Print
 
Uninstallation:
---------------

 1. Open a command prompt and navigate to SslPyFilter directory.
 2. > regsvr32 /u SslPyFilterMarshal.dll
 3. > del C:\Windows\System32\SslHook.dll
 4. > LspInstall.exe /Print
 5. > LspInstall.exe /Uninstall <id> [ Where <id> is the value from the line that contains "+IFS LSP \"SslLsp\"" ]

Usage:
------

 1. Locate pid of target application (for instance, using Sysinternals pslist).
 2. Open a command prompt and navigate to SslPyFilter directory.
 3. > rundll32 SslHook.dll,Inject <pid>
 4. > SslPyFilter.exe Filters\SslPyFilter.py

================================================================================
