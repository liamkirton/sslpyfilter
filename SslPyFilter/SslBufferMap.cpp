// ========================================================================================================================
// SslPyFilter
//
// Copyright ©2007 Liam Kirton <liam@int3.ws>
// ========================================================================================================================
// SslBufferMap.cpp
//
// Created: 30/11/2007
// ========================================================================================================================

#include "SslBufferMap.h"

#include <iostream>

// ========================================================================================================================

static SslBufferMap g_SslBufferMap;

// ========================================================================================================================

SslBufferMap::SslBufferMap()
{
	if((hMutex_ = CreateMutex(NULL, FALSE, NULL)) == NULL)
	{
		std::cout << "Error: CreateMutex() Failed." << std::endl;
		return;
	}
}

// ========================================================================================================================

SslBufferMap::~SslBufferMap()
{

}

// ========================================================================================================================

SslBufferMap *SslBufferMap::GetInstance()
{
	return &g_SslBufferMap;
}

// ========================================================================================================================

void SslBufferMap::Map(BSTR unencryptedBuffer, BSTR encryptedBuffer)
{
	BSTR newOriginal = SysAllocStringByteLen(reinterpret_cast<char *>(&(unencryptedBuffer[0])), SysStringByteLen(unencryptedBuffer));
	BSTR newEncrypted = SysAllocStringByteLen(reinterpret_cast<char *>(&(encryptedBuffer[0])), SysStringByteLen(encryptedBuffer));

	Lock();
	try
	{
		bufferMap_[newOriginal] = newEncrypted;
	}
	catch(...)
	{

	}
	Unlock();
}

// ========================================================================================================================

BSTR SslBufferMap::Unmap(BSTR unencryptedBuffer, DWORD &startOffset, DWORD &endOffset)
{
	BSTR encryptedBuffer = NULL;
	startOffset = endOffset = -1;

	char *pUnencrypted = reinterpret_cast<char *>(&(unencryptedBuffer[0]));
	DWORD dwUnencrypedLength = SysStringByteLen(unencryptedBuffer);

	Lock();

	try
	{
		for(std::map<BSTR, BSTR>::iterator i = bufferMap_.begin(); i != bufferMap_.end(); ++i)
		{
			char *pMapUnencrypted = reinterpret_cast<char *>(&(i->first[0]));
			DWORD dwMapUnencryptedLength = SysStringByteLen(i->first);

			for(DWORD j = 0; (j < dwUnencrypedLength); ++j)
			{
				startOffset = j;

				for(DWORD k = 0; (k < dwMapUnencryptedLength) && ((j + k) < dwUnencrypedLength); ++k)
				{
					if(pMapUnencrypted[k] != pUnencrypted[j + k])
					{
						startOffset = endOffset = -1;
						break;
					}
					endOffset = j + k + 1;
				}

				if(startOffset != -1)
				{
					break;
				}
			}
			
			if(startOffset != -1)
			{
				encryptedBuffer = i->second;
				SysFreeString(i->first);
				bufferMap_.erase(i);
				break;
			}
		}
	}
	catch(...)
	{
		encryptedBuffer = NULL;
	}

	Unlock();

	return encryptedBuffer;
}

// ========================================================================================================================

void SslBufferMap::Lock()
{
	if(WaitForSingleObject(hMutex_, 2500) != WAIT_OBJECT_0)
	{
		std::cout << "Warning: WaitForSingleObject(hMutex_) Failed." << std::endl;
	}
}

// ========================================================================================================================

void SslBufferMap::Unlock()
{
	ReleaseMutex(hMutex_);
}

// ========================================================================================================================
