// ========================================================================================================================
// SslPyFilter
//
// Copyright ©2007 Liam Kirton <liam@int3.ws>
// ========================================================================================================================
// PyInstance.cpp
//
// Created: 23/04/2007
// ========================================================================================================================

#include "PyInstance.h"

#include <iostream>

#include "Main.h"

// ========================================================================================================================

static PyInstance g_PyInstance;

// ========================================================================================================================

static PyMethodDef SslPyFilterMethods[] =
{
    {"set_encrypt_filter", PyInstance::SetEncryptFilter, METH_VARARGS, NULL},
	{"set_decrypt_filter", PyInstance::SetDecryptFilter, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}
};

// ========================================================================================================================

PyInstance::PyInstance() : pyEncryptFilter_(NULL),
						   pyDecryptFilter_(NULL)
{
	if((hMutex_ = CreateMutex(NULL, FALSE, NULL)) == NULL)
	{
		std::cout << "Error: CreateMutex() Failed." << std::endl;
		return;
	}
}

// ========================================================================================================================

PyInstance::~PyInstance()
{
	Unload();

	if(hMutex_ != NULL)
	{
		CloseHandle(hMutex_);
		hMutex_ = NULL;
	}
}

// ========================================================================================================================

void PyInstance::Load(const std::string &path)
{
	std::cout << "Loading \"" << path << "\"." << std::endl;

	__try
	{
		Lock();
	
		Py_Initialize();
		Py_InitModule("sslpyfilter", SslPyFilterMethods);
	
		HANDLE hPyFilter = INVALID_HANDLE_VALUE;
		if((hPyFilter = CreateFile(path.c_str(),
								   GENERIC_READ,
								   FILE_SHARE_READ,
								   NULL,
								   OPEN_EXISTING,
								   FILE_ATTRIBUTE_NORMAL,
								   NULL)) != INVALID_HANDLE_VALUE)
		{
			HANDLE hPyFilterMapping = NULL;
			if((hPyFilterMapping = CreateFileMapping(hPyFilter, NULL, PAGE_READONLY, 0, GetFileSize(hPyFilter, NULL), NULL)) != NULL)
			{
				char *pPyFilter = reinterpret_cast<char *>(MapViewOfFile(hPyFilterMapping, FILE_MAP_READ, 0, 0, 0));
				if(pPyFilter != NULL)
				{
					char *pPyBuffer = new char[GetFileSize(hPyFilter, NULL) + 1];
					RtlCopyMemory(pPyBuffer, pPyFilter, GetFileSize(hPyFilter, NULL));
					pPyBuffer[GetFileSize(hPyFilter, NULL)] = '\0';
					PyRun_SimpleString(pPyBuffer);
					delete [] pPyBuffer;
					
					if((pyEncryptFilter_ == NULL) || (pyDecryptFilter_ == NULL))
					{
						std::cout << "Error: Python SslPyFilter.set_encrypt_filter Or SslPyFilter.set_decrypt_filter Failed." << std::endl;
					}
	
					UnmapViewOfFile(pPyFilter);
				}
				else
				{
					std::cout << "Error: MapViewOfFile() Failed." << std::endl;
					return;
				}
				CloseHandle(hPyFilterMapping);
			}
			else
			{
				std::cout << "Error: CreateFileMapping() Failed." << std::endl;
			}
	
			CloseHandle(hPyFilter);
		}
		else
		{
			std::cout << "Error: CreateFile() Failed." << std::endl;
		}
	
		std::cout << std::endl;
	}
	__finally
	{
		Unlock();
	}
}

// ========================================================================================================================

void PyInstance::Unload()
{
	if(pyEncryptFilter_ != NULL)
	{
		Py_DECREF(pyEncryptFilter_);
		pyEncryptFilter_ = NULL;
	}
	if(pyDecryptFilter_ != NULL)
	{
		Py_DECREF(pyDecryptFilter_);
		pyDecryptFilter_ = NULL;
	}

	Py_Finalize();
}

// ========================================================================================================================

void PyInstance::EncryptMessageFilter(unsigned int process, BSTR *encryptBuffer, BSTR *modifiedEncryptBuffer)
{
	__try
	{
		Lock();

		char *pBstr = reinterpret_cast<char *>(&(*encryptBuffer[0]));
		unsigned int pyBufferLen = SysStringByteLen(*encryptBuffer);
		
		PyObject *arglist = Py_BuildValue("(I,s#,i)", process, pBstr, pyBufferLen, pyBufferLen);
		PyObject *result = PyEval_CallObject(pyEncryptFilter_, arglist);
		Py_DECREF(arglist);
		arglist = NULL;

		if(result != NULL)
		{
			if(result != Py_None)
			{
				PyObject *pReturnBuffer = NULL;
				unsigned int pReturnBufferLen = 0;
				
				if(PyArg_Parse(result, "s#", &pReturnBuffer, &pReturnBufferLen))
				{
					*modifiedEncryptBuffer = SysAllocStringByteLen(reinterpret_cast<char *>(pReturnBuffer), pReturnBufferLen);
				}
				else
				{
					*modifiedEncryptBuffer = NULL;
					PyErr_WriteUnraisable(pyEncryptFilter_);
				}
				PyErr_Clear();
			}

			Py_DECREF(result);
		}
		else
		{
			*modifiedEncryptBuffer = NULL;
			PyErr_WriteUnraisable(pyEncryptFilter_);
		}
	}
	__finally
	{
		Unlock();
	}
}

// ========================================================================================================================

void PyInstance::DecryptMessageFilter(unsigned int process, BSTR *decryptBuffer, BSTR *modifiedDecryptBuffer)
{
	__try
	{
		Lock();

		char *pBstr = reinterpret_cast<char *>(&(*decryptBuffer[0]));
		unsigned int pyBufferLen = SysStringByteLen(*decryptBuffer);
		
		PyObject *arglist = Py_BuildValue("(I,s#,i)", process, pBstr, pyBufferLen, pyBufferLen);
		PyObject *result = PyEval_CallObject(pyDecryptFilter_, arglist);
		Py_DECREF(arglist);
		arglist = NULL;

		if(result != NULL)
		{
			if(result != Py_None)
			{
				PyObject *pReturnBuffer = NULL;
				unsigned int pReturnBufferLen = 0;
				char fillChar = '\0';

				if(PyArg_ParseTuple(result, "s#c", &pReturnBuffer, &pReturnBufferLen, &fillChar))
				{
					if(pReturnBufferLen > pyBufferLen)
					{
						EnterCriticalSection(&g_ConsoleCriticalSection);
						std::cout << "PyInstance::DecryptMessageFilter() WARNING: Output Buffer Size > Input Buffer Size. Truncating." << std::endl;
						LeaveCriticalSection(&g_ConsoleCriticalSection);
					}

					*modifiedDecryptBuffer = SysAllocStringByteLen(NULL, pyBufferLen);
					pBstr = reinterpret_cast<char *>(&(*modifiedDecryptBuffer[0]));

					for(unsigned int i = 0; i < pyBufferLen; ++i)
					{
						if(i < pReturnBufferLen)
						{
							pBstr[i] = reinterpret_cast<char *>(pReturnBuffer)[i];
						}
						else
						{
							pBstr[i] = fillChar;
						}
					}
				}
				else
				{
					*modifiedDecryptBuffer = NULL;
					PyErr_WriteUnraisable(pyDecryptFilter_);
				}
				PyErr_Clear();
			}

			Py_DECREF(result);
		}
		else
		{
			*modifiedDecryptBuffer = NULL;
			PyErr_WriteUnraisable(pyDecryptFilter_);
		}
	}
	__finally
	{
		Unlock();
	}
}

// ========================================================================================================================

PyInstance *PyInstance::GetInstance()
{
	return &g_PyInstance;
}

// ========================================================================================================================

PyObject *PyInstance::SetEncryptFilter(PyObject *dummy, PyObject *args)
{
	PyObject *pyResult = NULL;

	__try
	{
		g_PyInstance.Lock();

		if(PyArg_ParseTuple(args, "O", &g_PyInstance.pyEncryptFilter_))
		{
			if(!PyCallable_Check(g_PyInstance.pyEncryptFilter_))
			{
				PyErr_SetString(PyExc_TypeError, "Error: SetEncryptFilter() - Parameter Must Be Callable.");
			}
			else
			{
				Py_XINCREF(g_PyInstance.pyEncryptFilter_); 
				Py_INCREF(Py_None);
				pyResult = Py_None;
			}
		}
	}
	__finally
	{
		g_PyInstance.Unlock();
	}
    return pyResult;
}

// ========================================================================================================================

PyObject *PyInstance::SetDecryptFilter(PyObject *dummy, PyObject *args)
{
	PyObject *pyResult = NULL;

	__try
	{
		g_PyInstance.Lock();
	
		if(PyArg_ParseTuple(args, "O", &g_PyInstance.pyDecryptFilter_))
		{
			if(!PyCallable_Check(g_PyInstance.pyDecryptFilter_))
			{
				PyErr_SetString(PyExc_TypeError, "Error: SetEncryptFilter() - Parameter Must Be Callable.");
			}
			else
			{
				Py_XINCREF(g_PyInstance.pyDecryptFilter_);
				Py_INCREF(Py_None);
				pyResult = Py_None;
			}
		}
	}
	__finally
	{
		g_PyInstance.Unlock();
	}
    return pyResult;
}

// ========================================================================================================================

void PyInstance::Lock()
{
	if(WaitForSingleObject(hMutex_, 2500) != WAIT_OBJECT_0)
	{
		std::cout << "Warning: WaitForSingleObject(hMutex_) Failed." << std::endl;
	}
}

// ========================================================================================================================

void PyInstance::Unlock()
{
	ReleaseMutex(hMutex_);
}

// ========================================================================================================================
