// ========================================================================================================================
// SslPyFilter
//
// Copyright ©2007-2008 Liam Kirton <liam@int3.ws>
// ========================================================================================================================
// SslPyFilter.cpp
//
// Created: 18/04/2007
// ========================================================================================================================

#include "SslPyFilter.h"

#include <iostream>

#include "Guid.h"
#include "PyInstance.h"
#include "SslBufferMap.h"

// ========================================================================================================================

SslPyFilter::SslPyFilter() : dwObjRefCount_(1)
{

}

// ========================================================================================================================

SslPyFilter::~SslPyFilter()
{

}

// ========================================================================================================================

STDMETHODIMP SslPyFilter::QueryInterface(REFIID iid, void **ppvObject)
{
	*ppvObject = NULL;
	HRESULT hResult = E_NOINTERFACE;

	if(IsEqualIID(iid, IID_IUnknown))
	{
		*ppvObject = this;
	}
	else if(IsEqualIID(iid, IID_ISslPyFilter))
	{
		*ppvObject = dynamic_cast<IUnknown *>(this);
	}
	
	if(*ppvObject != NULL)
	{
		AddRef();
		hResult = S_OK;
	}
	return hResult;
}

// ========================================================================================================================

STDMETHODIMP_(DWORD) SslPyFilter::AddRef()
{
	return ++dwObjRefCount_;
}

// ========================================================================================================================

STDMETHODIMP_(DWORD) SslPyFilter::Release()
{
	if(--dwObjRefCount_ == 0)
	{
		delete this;
		return 0;
	}
	return dwObjRefCount_;
}

// ========================================================================================================================

STDMETHODIMP SslPyFilter::EncryptMessagePrologueFilter(unsigned int process, unsigned int thread, BSTR *encryptBuffer, BSTR *modifiedEncryptBuffer)
{
	PyInstance *pyInstance = PyInstance::GetInstance();
	if(pyInstance != NULL)
	{
		pyInstance->EncryptMessageFilter(process, thread, encryptBuffer, modifiedEncryptBuffer);
	}
	else
	{
		std::cout << "Error: PyInstance::GetInstance() Failed." << std::endl;
	}
	return S_OK;
}

// ========================================================================================================================

STDMETHODIMP SslPyFilter::EncryptMessageEpilogueFilter(unsigned int process, unsigned int thread, BSTR *originalBuffer, BSTR *encryptBuffer)
{
	SslBufferMap *sslBufferMap = SslBufferMap::GetInstance();
	sslBufferMap->Map(*originalBuffer, *encryptBuffer);
	return S_OK;
}

// ========================================================================================================================

STDMETHODIMP SslPyFilter::DecryptMessageEpilogueFilter(unsigned int process, unsigned int thread, BSTR *decryptBuffer, BSTR *modifiedDecryptBuffer)
{
	PyInstance *pyInstance = PyInstance::GetInstance();
	if(pyInstance != NULL)
	{
		pyInstance->DecryptMessageFilter(process, thread, decryptBuffer, modifiedDecryptBuffer);
	}
	else
	{
		std::cout << "Error: PyInstance::GetInstance() Failed." << std::endl;
	}
	return S_OK;
}

// ========================================================================================================================

STDMETHODIMP SslPyFilter::RecvFilter(unsigned int process,
									 unsigned int thread,
									 unsigned int socket,
									 BSTR *recvBuffer,
									 BSTR *modifiedRecvBuffer,
									 unsigned int *remaining)
{
	return S_OK;
}

// ========================================================================================================================

STDMETHODIMP SslPyFilter::SendFilter(unsigned int process,
									 unsigned int thread,
									 unsigned int socket,
									 BSTR *sendBuffer,
									 BSTR *modifiedSendBuffer)
{
	SslBufferMap *sslBufferMap = SslBufferMap::GetInstance();
	
	DWORD startOffset = 0;
	DWORD endOffset = 0;

	BSTR encryptedBuffer = sslBufferMap->Unmap(*sendBuffer, startOffset, endOffset);
	if(encryptedBuffer != NULL)
	{
		DWORD dwEncryptedBufferCount = SysStringByteLen(encryptedBuffer);
		char *pEncryptedBuffer = reinterpret_cast<char *>(&(encryptedBuffer[0]));

		DWORD dwSendBufferLength = SysStringByteLen(*sendBuffer);

		DWORD dwModifiedSendBufferCount = 0;
		DWORD dwModifiedSendBufferLength = dwSendBufferLength - (endOffset - startOffset) + dwEncryptedBufferCount;
		*modifiedSendBuffer = SysAllocStringByteLen(NULL, dwModifiedSendBufferLength);

		char *pSendBuffer = reinterpret_cast<char *>(&(*sendBuffer[0]));
		char *pModifiedSendBuffer = reinterpret_cast<char *>(&(*modifiedSendBuffer[0]));		

		for(DWORD i = 0; i < startOffset; ++i)
		{
			pModifiedSendBuffer[dwModifiedSendBufferCount++] = pSendBuffer[i];
		}
		for(DWORD i = 0; i < dwEncryptedBufferCount; ++i)
		{
			pModifiedSendBuffer[dwModifiedSendBufferCount++] = pEncryptedBuffer[i];
		}
		for(DWORD i = endOffset; i < dwSendBufferLength; ++i)
		{
			pModifiedSendBuffer[dwModifiedSendBufferCount++] = pSendBuffer[i];
		}

		SysFreeString(encryptedBuffer);
		encryptedBuffer = NULL;
	}

	return S_OK;
}

// ========================================================================================================================
