// ========================================================================================================================
// SslPyFilter
//
// Copyright ©2007-2008 Liam Kirton <liam@int3.ws>
// ========================================================================================================================
// PyInstance.h
//
// Created: 23/04/2007
// ========================================================================================================================

#pragma once

// ========================================================================================================================

#include <windows.h>

#ifdef _DEBUG
#undef _DEBUG
#include <python.h>
#define _DEBUG
#else
#include <python.h>
#endif

#include <string>

// ========================================================================================================================

class PyInstance
{
public:
	PyInstance();
	~PyInstance();

	void Load(const std::string &path);
	void Unload();

	void EncryptMessageFilter(unsigned int process, unsigned int thread, BSTR *encryptBuffer, BSTR *modifiedEncryptBuffer);
	void DecryptMessageFilter(unsigned int process, unsigned int thread, BSTR *decryptBuffer, BSTR *modifiedDecryptBuffer);

	static PyInstance *GetInstance();
	static PyObject *PyInstance::SetEncryptFilter(PyObject *dummy, PyObject *args);
	static PyObject *PyInstance::SetDecryptFilter(PyObject *dummy, PyObject *args);

private:
	void Lock();
	void Unlock();

	HANDLE hMutex_;
	
	PyObject *pyEncryptFilter_;
	PyObject *pyDecryptFilter_;
};

// ========================================================================================================================
