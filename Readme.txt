================================================================================
SslPyFilter 0.2.1
Copyright ©2007 Liam Kirton <liam@int3.ws>

29th August 2007
http://int3.ws/
================================================================================

Overview:
---------

SslPyFilter is a tool that allows the capture and modification of cleartext
traffic before encryption/after decryption by the Windows SSL SSPI API.

DLL injection and API hooking techniques are used to capture the data, whilst
modification is performed by Python language scripts loaded dynamically by the
SslPyFilter.exe application.

This is very much a proof-of-concept tool.

Please email any comments, questions or suggestions to liam@int3.ws.

Limitations:
------------

Currently, the following limitations apply:

 - Modified data buffers cannot exceed the original in length.
 - Script filename is hardcoded.
 - No documentation or script examples provided.
 
Installation:
-------------

Requires: Python 2.5.x (http://www.python.org/).

 1. Open a command prompt and navigate to SslPyFilter directory.
 2. > copy /y SslHook.dll C:\Windows\System32
 3. > regsvr32 SslPyFilterMarshal.dll
 
Uninstallation:
---------------

 1. Open a command prompt and navigate to SslPyFilter directory.
 2. > regsvr32 /u SslPyFilterMarshal.dll
 3. > del C:\Windows\System32\SslHook.dll

Usage:
------

 1. Locate pid of target application (for instance, using Sysinternals pslist).
 2. Open a command prompt and navigate to SslPyFilter directory.
 3. > rundll32 SslHook.dll,Inject <pid>
 4. > SslPyFilter.exe Filters\SslPyFilter.py

================================================================================
