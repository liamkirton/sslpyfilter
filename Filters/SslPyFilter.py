# --------------------------------------------------------------------------------
# SslPyFilter
#
# Copyright ©2007 Liam Kirton <liam@int3.ws>
# --------------------------------------------------------------------------------
# SslPyFilter.py
#
# Created: 28/08/2007
# --------------------------------------------------------------------------------

import re
import sslpyfilter

# --------------------------------------------------------------------------------

def encrypt_filter(pid, buffer, length):
	buffer = buffer.replace("GET / HTTP/1.1", "GET /asdfthisisatest.html HTTP/1.1")
	return buffer

# --------------------------------------------------------------------------------

def decrypt_filter(pid, buffer, length):
	buffer = buffer.replace("Java", "J$V$")
	return buffer, ' '

# --------------------------------------------------------------------------------

if __name__ == '__main__':
    sslpyfilter.set_encrypt_filter(encrypt_filter)
    sslpyfilter.set_decrypt_filter(decrypt_filter)
    print '\"Filters\\SslPyFilter.py\" Loaded.'
    
# --------------------------------------------------------------------------------
