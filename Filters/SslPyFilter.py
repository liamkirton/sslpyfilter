# --------------------------------------------------------------------------------
# SslPyFilter
#
# Copyright ©2007-2008 Liam Kirton <liam@int3.ws>
# --------------------------------------------------------------------------------
# SslPyFilter.py
#
# Created: 28/08/2007
# --------------------------------------------------------------------------------

import re
import sslpyfilter

# --------------------------------------------------------------------------------

def encrypt_filter(pid, tid, b):
	print '>>>>> encrypt_filter(%d:%d, %d)' % (pid, tid, len(b))
	
	request_match = re.compile(r'^([A-Z]+)\s+(.*)\s+HTTP/\d\.\d').match(b)
	if request_match != None:
		b = re.compile('(User-Agent\\:\\s+)(.*)(\)[\r\n]*)', re.I | re.M).sub('\g<1>\g<2>; SslPyFilter - Copyright (C)2008 Liam Kirton (int3.ws)\g<3>', b)
		print '\n%s' % b
	
	return (b, ' ')

# --------------------------------------------------------------------------------

def decrypt_filter(pid, tid, b):
	print '<<<<< decrypt_filter(%d:%d, %d)' % (pid, tid, len(b))
	return (b, ' ')

# --------------------------------------------------------------------------------

if __name__ == '__main__':
	sslpyfilter.set_encrypt_filter(encrypt_filter)
	sslpyfilter.set_decrypt_filter(decrypt_filter)
	print '\"Filters\\SslPyFilter.py\" Loaded.'

# --------------------------------------------------------------------------------
