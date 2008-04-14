// ========================================================================================================================
// SslHook
//
// Copyright ©2007 Liam Kirton <liam@int3.ws>
// ========================================================================================================================
// SslHook.cpp
//
// Created: 17/04/2007
// ========================================================================================================================

#define SECURITY_WIN32

#include <windows.h>
#include <security.h>
#include <tlhelp32.h>

#include <exception>

#include "../SslPyFilter/Guid.h"
#include "../SslPyFilterMarshal/ISslPyFilter_i.c"
#include "../SslPyFilterMarshal/ISslPyFilter_h.h"

// ========================================================================================================================

void DllProcessAttach();

void __stdcall Inject(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow);
void InstallHooks();
void InstallPrologueHook(unsigned char *lpFunction, unsigned char *lpHook);
void InstallEpilogueHook(unsigned char *lpFunction, unsigned char *lpHook);

void __stdcall EncryptMessagePrologueHookProc(PCtxtHandle phContext, ULONG fQOP, PSecBufferDesc pMessage, ULONG MessageSeqNo);
void __stdcall EncryptMessageEpilogueHookProc(PCtxtHandle phContext, ULONG fQOP, PSecBufferDesc pMessage, ULONG MessageSeqNo);
void __stdcall DecryptMessagePrologueHookProc(PCtxtHandle phContext, PSecBufferDesc pMessage, ULONG MessageSeqNo, PULONG pfQOP);
void __stdcall DecryptMessageEpilogueHookProc(PCtxtHandle phContext, PSecBufferDesc pMessage, ULONG MessageSeqNo, PULONG pfQOP);

DWORD WINAPI ComCallThreadProc(LPVOID lpParameter);

// ========================================================================================================================

typedef struct _COM_CALL
{
	unsigned short Type;
	
	BSTR *Buffer1;
	BSTR *Buffer2;

	HRESULT hResult;
} COM_CALL;

// ------------------------------------------------------------------------------------------------------------------------

typedef enum _COM_CALL_TYPE
{
	Type_EncryptMessagePrologueFilter,
	Type_EncryptMessageEpilogueFilter,
	Type_DecryptMessagePrologueFilter,
	Type_DecryptMessageEpilogueFilter
} COM_CALL_TYPE;

// ========================================================================================================================

HANDLE g_ComCallThread = NULL;
HANDLE g_ComCallThreadSignalEvent = NULL;
HANDLE g_ComCallThreadCompletionEvent = NULL;
HANDLE g_ComCallThreadMutex = NULL;
HANDLE g_ComCallThreadExitEvent = NULL;

bool g_ComCallAvailable = false;
COM_CALL *g_ComCallParameter = NULL;

// ========================================================================================================================

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DllProcessAttach();
		break;

	case DLL_PROCESS_DETACH:
		break;

	default:
		break;
	}
	return TRUE;
}

// ========================================================================================================================

void DllProcessAttach()
{
	char moduleFileName[MAX_PATH + 1];
	DWORD dwModuleFileNameLength = GetModuleFileName(GetModuleHandle(NULL), moduleFileName, MAX_PATH);
    
	bool bInRundll32 = false;
	for(DWORD i = dwModuleFileNameLength; i >= 0; --i)
	{
		if((moduleFileName[i] == '\\') && ((i + 12) < dwModuleFileNameLength))
		{
			if(((moduleFileName[i + 1] == 'r') || (moduleFileName[i + 1] == 'R')) &&
			   ((moduleFileName[i + 2] == 'u') || (moduleFileName[i + 2] == 'U')) &&
			   ((moduleFileName[i + 3] == 'n') || (moduleFileName[i + 3] == 'N')) &&
			   ((moduleFileName[i + 4] == 'd') || (moduleFileName[i + 4] == 'D')) &&
			   ((moduleFileName[i + 5] == 'l') || (moduleFileName[i + 5] == 'L')) &&
			   ((moduleFileName[i + 6] == 'l') || (moduleFileName[i + 6] == 'L')) &&
			   ((moduleFileName[i + 7] == '3') || (moduleFileName[i + 7] == '3')) &&
			   ((moduleFileName[i + 8] == '2') || (moduleFileName[i + 8] == '2')) &&
			   (moduleFileName[i + 9] == '.') &&
			   ((moduleFileName[i + 10] == 'e') || (moduleFileName[i + 10] == 'E')) &&
			   ((moduleFileName[i + 11] == 'x') || (moduleFileName[i + 11] == 'X')) &&
			   ((moduleFileName[i + 12] == 'e') || (moduleFileName[i + 12] == 'E')))
			{
				bInRundll32 = true;
			}
			break;
		}
	}

	if(!bInRundll32)
	{
		InstallHooks();
	}
}

// ========================================================================================================================

void __stdcall Inject(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
	DWORD dwProcessID = strtol(lpszCmdLine, NULL, 10);

	HANDLE hProcessSnapshot = NULL;
	HANDLE hProcess = NULL;

	try
	{
		if((hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) == INVALID_HANDLE_VALUE)
		{
			hProcessSnapshot = NULL;
			throw std::exception("CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS) Failed.");
		}
 
		PROCESSENTRY32 processEntry32;
		ZeroMemory(&processEntry32, sizeof(PROCESSENTRY32));
		processEntry32.dwSize = sizeof(PROCESSENTRY32);
	
		if(Process32First(hProcessSnapshot, &processEntry32))
		{
			do
			{
				if(processEntry32.th32ProcessID == dwProcessID)
				{
					if((hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID)) == NULL)
					{
						throw std::exception("OpenProcess(PROCESS_ALL_ACCESS) Failed.");
					}

					LPVOID lpProcParamMem = NULL;
					if((lpProcParamMem = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT, PAGE_READWRITE)) == NULL)
					{
						throw std::exception("VirtualAllocEx() Failed.");
					}
					
					if(!WriteProcessMemory(hProcess, lpProcParamMem, "SslHook.dll\0", 12, NULL))
					{
						throw std::exception("WriteProcessMemory() Failed.");
					}

					FARPROC fpLoadLibrary = GetProcAddress(GetModuleHandle("kernel32"), "LoadLibraryA");
					if(CreateRemoteThread(hProcess, NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(fpLoadLibrary), lpProcParamMem, 0, NULL) == NULL)
					{
						throw std::exception("CreateRemoteThread() Failed.");
					}

					break;
				}
			}
			while(Process32Next(hProcessSnapshot, &processEntry32));
		}
	}
	catch(const std::exception &e)
	{
		MessageBox(NULL, e.what(), "SslHook Error", MB_ICONEXCLAMATION);
	}

	if(hProcess != NULL)
	{
		CloseHandle(hProcess);
		hProcess = NULL;
	}
	else
	{
		MessageBox(NULL, "Invalid Process Identifier", "SslHook Error", MB_ICONEXCLAMATION);
	}

	if(hProcessSnapshot != NULL)
	{
		CloseHandle(hProcessSnapshot);
		hProcessSnapshot = NULL;
	}
}

// ========================================================================================================================

static void __declspec(naked) EncryptMessagePrologueHook()
{
	__asm
	{
		push ebp
		mov ebp, esp
		pushad

		mov eax, dword ptr[ebp+0x08]
		mov ebx, dword ptr[ebp+0x0C]
		mov ecx, dword ptr[ebp+0x10]
		mov edx, dword ptr[ebp+0x14]

		push edx
		push ecx
		push ebx
		push eax
		call EncryptMessagePrologueHookProc

		popad
		
		_emit 0xE9 ; JMP
		_emit 0x90
		_emit 0x90
		_emit 0x90
		_emit 0x90
	}
}

// ========================================================================================================================

static void __declspec(naked) EncryptMessageEpilogueHook()
{
	__asm
	{
		_emit 0xC9 ; LEAVE

		push ebp
		mov ebp, esp
		pushad

		mov eax, dword ptr[ebp+0x08]
		mov ebx, dword ptr[ebp+0x0C]
		mov ecx, dword ptr[ebp+0x10]
		mov edx, dword ptr[ebp+0x14]

		push edx
		push ecx
		push ebx
		push eax
		call EncryptMessageEpilogueHookProc

		popad
		pop ebp

		ret 0x0010
	}
}

// ========================================================================================================================

static void __declspec(naked) DecryptMessagePrologueHook()
{
	__asm
	{
		push ebp
		mov ebp, esp
		pushad

		mov eax, dword ptr[ebp+0x08]
		mov ebx, dword ptr[ebp+0x0C]
		mov ecx, dword ptr[ebp+0x10]
		mov edx, dword ptr[ebp+0x14]

		push edx
		push ecx
		push ebx
		push eax
		call DecryptMessagePrologueHookProc

		popad

		_emit 0xE9 ; JMP
		_emit 0x90
		_emit 0x90
		_emit 0x90
		_emit 0x90
	}
}

// ========================================================================================================================

static void __declspec(naked) DecryptMessageEpilogueHook()
{
	__asm
	{
		_emit 0xC9 ; LEAVE

		push ebp
		mov ebp, esp
		pushad

		mov eax, dword ptr[ebp+0x08]
		mov ebx, dword ptr[ebp+0x0C]
		mov ecx, dword ptr[ebp+0x10]
		mov edx, dword ptr[ebp+0x14]

		push edx
		push ecx
		push ebx
		push eax
		call DecryptMessageEpilogueHookProc

		popad
		pop ebp

		ret 0x0010
	}
}

// ========================================================================================================================

void InstallHooks()
{
	HMODULE hSecur32Module = NULL;
	FARPROC fpEncryptMessage = NULL;
	FARPROC fpDecryptMessage = NULL;

	try
	{
		if((g_ComCallThreadSignalEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
		{
			throw std::exception("CreateEvent() Failed.");
		}

		if((g_ComCallThreadCompletionEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
		{
			throw std::exception("CreateEvent() Failed.");
		}

		if((g_ComCallThreadMutex = CreateMutex(NULL, FALSE, NULL)) == NULL)
		{
			throw std::exception("CreateEvent() Failed.");
		}

		if((g_ComCallThreadExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
		{
			throw std::exception("CreateEvent() Failed.");
		}

		if((g_ComCallThread = CreateThread(NULL, 0, ComCallThreadProc, NULL, 0, NULL)) == NULL)
		{
			throw std::exception("CreateThread() Failed.");
		}

		if((hSecur32Module = GetModuleHandle("secur32.dll")) == NULL)
		{
			throw std::exception("secur32.dll Not Loaded By Process.");
		}

		if((fpEncryptMessage = GetProcAddress(hSecur32Module, "EncryptMessage")) == NULL) // RVA 0000A5CF
		{
			throw std::exception("GetProcAddress(\"EncryptMessage\") Failed.");
		}

		if((fpDecryptMessage = GetProcAddress(hSecur32Module, "DecryptMessage")) == NULL) // RVA 0000A61E
		{
			throw std::exception("GetProcAddress(\"DecryptMessage\") Failed.");
		}

		EncryptMessage

		unsigned char *lpEncryptMessage = reinterpret_cast<unsigned char *>(fpEncryptMessage);
		unsigned char *lpEncryptMessagePrologueHook = reinterpret_cast<unsigned char *>(EncryptMessagePrologueHook);
		unsigned char *lpEncryptMessageEpilogueHook = reinterpret_cast<unsigned char *>(EncryptMessageEpilogueHook);

		InstallPrologueHook(lpEncryptMessage, lpEncryptMessagePrologueHook);
		InstallEpilogueHook(lpEncryptMessage, lpEncryptMessageEpilogueHook);
		
		unsigned char *lpDecryptMessage = reinterpret_cast<unsigned char *>(fpDecryptMessage);
		unsigned char *lpDecryptMessagePrologueHook = reinterpret_cast<unsigned char *>(DecryptMessagePrologueHook);
		unsigned char *lpDecryptMessageEpilogueHook = reinterpret_cast<unsigned char *>(DecryptMessageEpilogueHook);

		InstallPrologueHook(lpDecryptMessage, lpDecryptMessagePrologueHook);
		InstallEpilogueHook(lpDecryptMessage, lpDecryptMessageEpilogueHook);

		fpEncryptMessage = NULL;
		fpDecryptMessage = NULL;
	}
	catch(const std::exception &e)
	{
		MessageBox(NULL, e.what(), "SslHook Error", MB_ICONEXCLAMATION);
	}
}

// ========================================================================================================================

void InstallPrologueHook(unsigned char *lpFunction, unsigned char *lpHook)
{
	DWORD dwOldFunctionProtection;
	DWORD dwOldHookProtection;
	
	if(!VirtualProtect(lpFunction, 4096, PAGE_EXECUTE_READWRITE, &dwOldFunctionProtection))
	{
		throw std::exception("VirtualProtect() Failed.");
	}

	if(!VirtualProtect(lpHook, 4096, PAGE_EXECUTE_READWRITE, &dwOldHookProtection))
	{
		throw std::exception("VirtualProtect() Failed.");
	}

	lpFunction[0] = 0xE9;
	DWORD *pFunctionPrologueHookRel32 = reinterpret_cast<DWORD *>(&lpFunction[1]);
	*pFunctionPrologueHookRel32 = static_cast<DWORD>(lpHook -
													 reinterpret_cast<unsigned char *>(pFunctionPrologueHookRel32) - 4);
	
	while(true)
	{
		if((lpHook[0] == 0xE9) &&
		   (lpHook[1] == 0x90) &&
		   (lpHook[2] == 0x90) &&
		   (lpHook[3] == 0x90) &&
		   (lpHook[4] == 0x90))
		{
			DWORD *pFunctionPrologueRel32 = reinterpret_cast<DWORD *>(&lpHook[1]);
			*pFunctionPrologueRel32 = static_cast<DWORD>((lpFunction + 5) - 
														 reinterpret_cast<unsigned char *>(pFunctionPrologueRel32) - 4);
			break;
		}
		lpHook++;
	}

	if(!VirtualProtect(lpFunction, 4096, dwOldFunctionProtection, &dwOldFunctionProtection))
	{
		throw std::exception("VirtualProtect() Failed.");
	}

	if(!VirtualProtect(lpHook, 4096, dwOldHookProtection, &dwOldHookProtection))
	{
		throw std::exception("VirtualProtect() Failed.");
	}

	FlushInstructionCache(GetCurrentProcess(), lpFunction, 4096);
	FlushInstructionCache(GetCurrentProcess(), lpHook, 4096);
}

// ========================================================================================================================

void InstallEpilogueHook(unsigned char *lpFunction, unsigned char *lpHook)
{
	DWORD dwOldFunctionProtection;

	if(!VirtualProtect(lpFunction, 4096, PAGE_EXECUTE_READWRITE, &dwOldFunctionProtection))
	{
		throw std::exception("VirtualProtect() Failed.");
	}

	while(true)
	{
		if((lpFunction[0] == 0xC9) &&
		   (lpFunction[1] == 0xC2) &&
		   (lpFunction[2] == 0x10) &&
		   (lpFunction[3] == 0x00))
		{
			lpFunction[0] = 0xE9;

			DWORD *pHookRel32 = reinterpret_cast<DWORD *>(&lpFunction[1]);
			*pHookRel32 = static_cast<DWORD>(lpHook -
											 reinterpret_cast<unsigned char *>(pHookRel32) - 4);

			break;
		}
		lpFunction++;
	}

	if(!VirtualProtect(lpFunction, 4096, dwOldFunctionProtection, &dwOldFunctionProtection))
	{
		throw std::exception("VirtualProtect() Failed.");
	}

	FlushInstructionCache(GetCurrentProcess(), lpFunction, 4096);
}

// ========================================================================================================================

void __stdcall EncryptMessagePrologueHookProc(PCtxtHandle phContext, ULONG fQOP, PSecBufferDesc pMessage, ULONG MessageSeqNo)
{
	if(g_ComCallAvailable)
	{
		for(unsigned long i = 0; i < pMessage->cBuffers; ++i)
		{
			if(pMessage->pBuffers[i].BufferType == SECBUFFER_DATA)
			{
				BSTR encryptBuffer = SysAllocStringByteLen(reinterpret_cast<char *>(pMessage->pBuffers[i].pvBuffer), pMessage->pBuffers[i].cbBuffer);
				BSTR modifiedEncryptBuffer = NULL;
				
				COM_CALL comCall;
				comCall.Type = Type_EncryptMessagePrologueFilter;
				comCall.Buffer1 = &encryptBuffer;
				comCall.Buffer2 = &modifiedEncryptBuffer;

				if(WaitForSingleObject(g_ComCallThreadMutex, 5000) == WAIT_OBJECT_0)
				{
					g_ComCallParameter = &comCall;
					SetEvent(g_ComCallThreadSignalEvent);

					if(WaitForSingleObject(g_ComCallThreadCompletionEvent, 5000) == WAIT_OBJECT_0)
					{
						if(SUCCEEDED(g_ComCallParameter->hResult))
						{
							char *pSrcBuffer = (modifiedEncryptBuffer != NULL) ? reinterpret_cast<char *>(&modifiedEncryptBuffer[0]) :
																				 reinterpret_cast<char *>(pMessage->pBuffers[i].pvBuffer);

							DWORD cbNewBuffer = (modifiedEncryptBuffer != NULL) ? SysStringByteLen(modifiedEncryptBuffer) :
																				  pMessage->pBuffers[i].cbBuffer;
							char *pNewBuffer = reinterpret_cast<char *>(VirtualAlloc(NULL, pMessage->pBuffers[i].cbBuffer + 12, MEM_COMMIT, PAGE_READWRITE));

							RtlCopyMemory(pNewBuffer, pSrcBuffer, cbNewBuffer);
							*reinterpret_cast<DWORD *>(pNewBuffer + cbNewBuffer) = 0x4d41494c;
							*reinterpret_cast<DWORD *>(pNewBuffer + cbNewBuffer + 4) = reinterpret_cast<DWORD>(pMessage->pBuffers[i].pvBuffer);
							*reinterpret_cast<DWORD *>(pNewBuffer + cbNewBuffer + 8) = static_cast<DWORD>(pMessage->pBuffers[i].cbBuffer);

							if(modifiedEncryptBuffer != NULL)
							{
								SysFreeString(modifiedEncryptBuffer);
								modifiedEncryptBuffer = NULL;
							}

							pMessage->pBuffers[i].cbBuffer = cbNewBuffer;
							pMessage->pBuffers[i].pvBuffer = pNewBuffer;
						}
					}

					ReleaseMutex(g_ComCallThreadMutex);
				}

				SysFreeString(encryptBuffer);
				encryptBuffer = NULL;
			}
		}
	}
}

// ========================================================================================================================

void __stdcall EncryptMessageEpilogueHookProc(PCtxtHandle phContext, ULONG fQOP, PSecBufferDesc pMessage, ULONG MessageSeqNo)
{
	if(g_ComCallAvailable)
	{
		for(unsigned long i = 0; i < pMessage->cBuffers; ++i)
		{
			if(pMessage->pBuffers[i].BufferType == SECBUFFER_DATA)
			{
				char *pSrcBuffer = reinterpret_cast<char *>(pMessage->pBuffers[i].pvBuffer);

				DWORD dwMagic = *reinterpret_cast<DWORD *>(pSrcBuffer + pMessage->pBuffers[i].cbBuffer);
				if(dwMagic == 0x4d41494c)
				{
					BSTR encryptedBuffer = SysAllocStringByteLen(reinterpret_cast<char *>(pMessage->pBuffers[i].pvBuffer), pMessage->pBuffers[i].cbBuffer);
					BSTR originalBuffer = NULL;

					pMessage->pBuffers[i].pvBuffer = reinterpret_cast<void *>(*reinterpret_cast<DWORD *>(pSrcBuffer + pMessage->pBuffers[i].cbBuffer + 4));
					pMessage->pBuffers[i].cbBuffer = *reinterpret_cast<DWORD *>(pSrcBuffer + pMessage->pBuffers[i].cbBuffer + 8);

					originalBuffer = SysAllocStringByteLen(reinterpret_cast<char *>(pMessage->pBuffers[i].pvBuffer), pMessage->pBuffers[i].cbBuffer);

					COM_CALL comCall;
					comCall.Type = Type_EncryptMessageEpilogueFilter;
					comCall.Buffer1 = &originalBuffer;
					comCall.Buffer2 = &encryptedBuffer;

					if(WaitForSingleObject(g_ComCallThreadMutex, 5000) == WAIT_OBJECT_0)
					{
						g_ComCallParameter = &comCall;
						SetEvent(g_ComCallThreadSignalEvent);

						if(WaitForSingleObject(g_ComCallThreadCompletionEvent, 5000) == WAIT_OBJECT_0)
						{
							if(SUCCEEDED(g_ComCallParameter->hResult))
							{
								
							}
						}

						ReleaseMutex(g_ComCallThreadMutex);
					}

					SysFreeString(encryptedBuffer);
					SysFreeString(originalBuffer);

					VirtualFree(pSrcBuffer, 0, MEM_RELEASE);
					pSrcBuffer = NULL;
				}
			}
		}
	}
}

// ========================================================================================================================

void __stdcall DecryptMessagePrologueHookProc(PCtxtHandle phContext, PSecBufferDesc pMessage, ULONG MessageSeqNo, PULONG pfQOP)
{
	
}

// ========================================================================================================================

void __stdcall DecryptMessageEpilogueHookProc(PCtxtHandle phContext, PSecBufferDesc pMessage, ULONG MessageSeqNo, PULONG pfQOP)
{
	if(g_ComCallAvailable)
	{
		for(unsigned long i = 0; i < pMessage->cBuffers; ++i)
		{
			if(pMessage->pBuffers[i].BufferType == SECBUFFER_DATA)
			{
				BSTR decryptBuffer = SysAllocStringByteLen(reinterpret_cast<char *>(pMessage->pBuffers[i].pvBuffer), pMessage->pBuffers[i].cbBuffer);
				BSTR modifiedDecryptBuffer = NULL;

				COM_CALL comCall;
				comCall.Type = Type_DecryptMessageEpilogueFilter;
				comCall.Buffer1 = &decryptBuffer;
				comCall.Buffer2 = &modifiedDecryptBuffer;

				if(WaitForSingleObject(g_ComCallThreadMutex, 5000) == WAIT_OBJECT_0)
				{
					g_ComCallParameter = &comCall;

					SetEvent(g_ComCallThreadSignalEvent);

					if(WaitForSingleObject(g_ComCallThreadCompletionEvent, 5000) == WAIT_OBJECT_0)
					{
						if(SUCCEEDED(g_ComCallParameter->hResult))
						{
							if(modifiedDecryptBuffer != NULL)
							{
								DWORD dwBufferLength = max(SysStringByteLen(modifiedDecryptBuffer), pMessage->pBuffers[i].cbBuffer);
								RtlCopyMemory(pMessage->pBuffers[i].pvBuffer, reinterpret_cast<char *>(&modifiedDecryptBuffer[0]), dwBufferLength);
								SysFreeString(modifiedDecryptBuffer);
								modifiedDecryptBuffer = NULL;
							}
						}
					}

					ReleaseMutex(g_ComCallThreadMutex);
				}

				SysFreeString(decryptBuffer);
				decryptBuffer = NULL;
			}
		}
	}
}

// ========================================================================================================================

DWORD WINAPI ComCallThreadProc(LPVOID lpParameter)
{
	ISslPyFilter *pSslPyFilter = NULL;

	HRESULT hInitCom = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if((hInitCom == S_OK) || (hInitCom == S_FALSE) || (hInitCom == RPC_E_CHANGED_MODE))
	{
		while(WaitForSingleObject(g_ComCallThreadExitEvent, 0) != WAIT_OBJECT_0)
		{
			g_ComCallAvailable = false;

			IUnknown *pClassFactoryUnknown = NULL;
			if(SUCCEEDED(CoGetClassObject(CLSID_SslPyFilter,
										  CLSCTX_LOCAL_SERVER,
										  NULL,
										  IID_IUnknown,
										  reinterpret_cast<LPVOID *>(&pClassFactoryUnknown))))
			{
				IClassFactory *pClassFactory = NULL;
				if(SUCCEEDED(pClassFactoryUnknown->QueryInterface(IID_IClassFactory,
																  reinterpret_cast<LPVOID *>(&pClassFactory))))
				{
					IUnknown *pSslPyFilterUnknown = NULL;
					if(SUCCEEDED(pClassFactory->CreateInstance(NULL,
															   IID_IUnknown,
															   reinterpret_cast<LPVOID *>(&pSslPyFilterUnknown))))
					{
						if(FAILED(pSslPyFilterUnknown->QueryInterface(IID_ISslPyFilter,
																	  reinterpret_cast<LPVOID *>(&pSslPyFilter))))
						{
							OutputDebugString("<SslPyFilterProvider::ComCallThreadProc()> WARNING: pSslPyFilterUnknown->QueryInterface() Failed.\n");
							pSslPyFilter = NULL;
						}
						else
						{
							OutputDebugString("<SslPyFilterProvider::ComCallThreadProc()> pSslPyFilterUnknown->QueryInterface() Succeeded.\n");
						}

						pSslPyFilterUnknown->Release();
						pSslPyFilterUnknown = NULL;
					}
					else
					{
						OutputDebugString("<SslPyFilterProvider::ComCallThreadProc()> WARNING: pClassFactory->CreateInstance() Failed.\n");
					}
					pClassFactory->Release();
					pClassFactory = NULL;
				}
				else
				{
					OutputDebugString("<SslPyFilterProvider::ComCallThreadProc()> WARNING: pClassFactoryUnknown->QueryInterface() Failed.\n");
				}
				pClassFactoryUnknown->Release();
				pClassFactoryUnknown = NULL;
			}
			else
			{
				OutputDebugString("<SslPyFilterProvider::ComCallThreadProc()> WARNING: CoGetClassObject(CLSID_SslPyFilter, IID_IUnknown) Failed.\n");
			}

			if(pSslPyFilter == NULL)
			{
				Sleep(2500);
			}
			else
			{
				ResetEvent(g_ComCallThreadCompletionEvent);
				ResetEvent(g_ComCallThreadSignalEvent);

				g_ComCallAvailable = true;

				HANDLE hWaitHandles[2];
				hWaitHandles[0] = g_ComCallThreadExitEvent;
				hWaitHandles[1] = g_ComCallThreadSignalEvent;

				while(true)
				{
					bool bInCallLoop = false;
					switch(WaitForMultipleObjects(sizeof(hWaitHandles) / sizeof(HANDLE),
												  reinterpret_cast<const HANDLE *>(&hWaitHandles),
												  FALSE,
												  INFINITE))
					{
						case WAIT_OBJECT_0:
							break;

						case WAIT_OBJECT_0 + 1:
							bInCallLoop = true;
							break;

						default:
							break;
					}
					if(!bInCallLoop)
					{
						break;
					}

					switch(g_ComCallParameter->Type)
					{
						case Type_EncryptMessagePrologueFilter:
							g_ComCallParameter->hResult = pSslPyFilter->EncryptMessagePrologueFilter(GetCurrentProcessId(),
																								     g_ComCallParameter->Buffer1,
																								     g_ComCallParameter->Buffer2);
							break;
						
						case Type_EncryptMessageEpilogueFilter:
							g_ComCallParameter->hResult = pSslPyFilter->EncryptMessageEpilogueFilter(GetCurrentProcessId(),
																								     g_ComCallParameter->Buffer1,
																									 g_ComCallParameter->Buffer2);
							break;

						case Type_DecryptMessagePrologueFilter:
							break;

						case Type_DecryptMessageEpilogueFilter:
							g_ComCallParameter->hResult = pSslPyFilter->DecryptMessageEpilogueFilter(GetCurrentProcessId(),
																								     g_ComCallParameter->Buffer1,
																								     g_ComCallParameter->Buffer2);
							break;

						default:
							OutputDebugString("<SslPyFilterProvider::ComCallThreadProc()> WARNING: g_ComCallParameter->Type Unknown.\n");
							break;
					}

					if(g_ComCallParameter->hResult == HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE))
					{
						OutputDebugString("<SslPyFilterProvider::ComCallThreadProc()> WARNING: pSslPyFilter Call Returned RPC_S_SERVER_UNAVAILABLE.\n");
						SetEvent(g_ComCallThreadCompletionEvent);
						break;
					}
					else
					{
						SetEvent(g_ComCallThreadCompletionEvent);
					}
				}

				pSslPyFilter->Release();
				pSslPyFilter = NULL;				
			}
		}
	}
	else
	{
		OutputDebugString("<SslPyFilterProvider::ComCallThreadProc()> WARNING: CoInitializeEx() Failed.\n");
	}

	if(SUCCEEDED(hInitCom))
	{
		CoUninitialize();
	}

	return 0;
}

// ========================================================================================================================
