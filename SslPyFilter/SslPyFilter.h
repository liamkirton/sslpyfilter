// ========================================================================================================================
// SslPyFilter
//
// Copyright ©2007 Liam Kirton <liam@int3.ws>
// ========================================================================================================================
// SslPyFilter.h
//
// Created: 18/04/2007
// ========================================================================================================================

#pragma once

// ========================================================================================================================

#include <windows.h>

#include "../SslPyFilterMarshal/ISslPyFilter_h.h"

// ========================================================================================================================

class SslPyFilter : public ISslPyFilter
{
public:
	SslPyFilter();
	~SslPyFilter();

	STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject);
	STDMETHODIMP_(DWORD) AddRef();
	STDMETHODIMP_(DWORD) Release();

	STDMETHODIMP EncryptMessagePrologueFilter(unsigned int process, BSTR *encryptBuffer, BSTR *modifiedEncryptBuffer);
	STDMETHODIMP EncryptMessageEpilogueFilter(unsigned int process, BSTR *unencryptedBuffer, BSTR *encryptedBuffer);

	STDMETHODIMP DecryptMessageEpilogueFilter(unsigned int process, BSTR *decryptBuffer, BSTR *modifiedDecryptBuffer);

	STDMETHODIMP RecvFilter(unsigned int process,
							unsigned int thread,
							unsigned int socket,
							BSTR *recvBuffer,
							BSTR *modifiedRecvBuffer,
							unsigned int *remaining);
	STDMETHODIMP SendFilter(unsigned int process,
							unsigned int thread,
							unsigned int socket,
							BSTR *sendBuffer,
							BSTR *modifiedSendBuffer);

private:
	DWORD dwObjRefCount_;
};

// ========================================================================================================================
