// ========================================================================================================================
// SslPyFilter
//
// Copyright ©2007 Liam Kirton <liam@int3.ws>
// ========================================================================================================================
// Guid.h
//
// Created: 18/04/2007
// ========================================================================================================================

#pragma once

// ========================================================================================================================

#include <windows.h>

// ========================================================================================================================

class ClassFactory : public IClassFactory
{
public:
	ClassFactory(CLSID clsid);
	virtual ~ClassFactory();
	
	STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject);
	STDMETHODIMP_(DWORD) AddRef();
	STDMETHODIMP_(DWORD) Release();

	STDMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject);
	STDMETHODIMP LockServer(BOOL fLock);

private:
	CLSID clsid_;
	DWORD dwObjRefCount_;
};

// ========================================================================================================================
