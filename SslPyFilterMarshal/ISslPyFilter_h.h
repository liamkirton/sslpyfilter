

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Mon Apr 14 19:36:37 2008
 */
/* Compiler settings for .\ISslPyFilter.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __ISslPyFilter_h_h__
#define __ISslPyFilter_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ISslPyFilter_FWD_DEFINED__
#define __ISslPyFilter_FWD_DEFINED__
typedef interface ISslPyFilter ISslPyFilter;
#endif 	/* __ISslPyFilter_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ISslPyFilter_INTERFACE_DEFINED__
#define __ISslPyFilter_INTERFACE_DEFINED__

/* interface ISslPyFilter */
/* [oleautomation][uuid][object] */ 


EXTERN_C const IID IID_ISslPyFilter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A3D201DE-0DDD-497a-B3DC-38048A9236C1")
    ISslPyFilter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EncryptMessagePrologueFilter( 
            /* [in] */ unsigned int process,
            /* [in] */ unsigned int thread,
            /* [in] */ BSTR *encryptBuffer,
            /* [out] */ BSTR *modifiedEncryptBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EncryptMessageEpilogueFilter( 
            /* [in] */ unsigned int process,
            /* [in] */ unsigned int thread,
            /* [in] */ BSTR *unencryptedBuffer,
            /* [in] */ BSTR *encryptedBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DecryptMessageEpilogueFilter( 
            /* [in] */ unsigned int process,
            /* [in] */ unsigned int thread,
            /* [in] */ BSTR *decryptBuffer,
            /* [out] */ BSTR *modifiedDecryptBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RecvFilter( 
            /* [in] */ unsigned int process,
            /* [in] */ unsigned int thread,
            /* [in] */ unsigned int socket,
            /* [in] */ BSTR *recvBuffer,
            /* [out] */ BSTR *modifiedRecvBuffer,
            /* [out] */ unsigned int *remaining) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SendFilter( 
            /* [in] */ unsigned int process,
            /* [in] */ unsigned int thread,
            /* [in] */ unsigned int socket,
            /* [in] */ BSTR *sendBuffer,
            /* [out] */ BSTR *modifiedSendBuffer) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISslPyFilterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISslPyFilter * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISslPyFilter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISslPyFilter * This);
        
        HRESULT ( STDMETHODCALLTYPE *EncryptMessagePrologueFilter )( 
            ISslPyFilter * This,
            /* [in] */ unsigned int process,
            /* [in] */ unsigned int thread,
            /* [in] */ BSTR *encryptBuffer,
            /* [out] */ BSTR *modifiedEncryptBuffer);
        
        HRESULT ( STDMETHODCALLTYPE *EncryptMessageEpilogueFilter )( 
            ISslPyFilter * This,
            /* [in] */ unsigned int process,
            /* [in] */ unsigned int thread,
            /* [in] */ BSTR *unencryptedBuffer,
            /* [in] */ BSTR *encryptedBuffer);
        
        HRESULT ( STDMETHODCALLTYPE *DecryptMessageEpilogueFilter )( 
            ISslPyFilter * This,
            /* [in] */ unsigned int process,
            /* [in] */ unsigned int thread,
            /* [in] */ BSTR *decryptBuffer,
            /* [out] */ BSTR *modifiedDecryptBuffer);
        
        HRESULT ( STDMETHODCALLTYPE *RecvFilter )( 
            ISslPyFilter * This,
            /* [in] */ unsigned int process,
            /* [in] */ unsigned int thread,
            /* [in] */ unsigned int socket,
            /* [in] */ BSTR *recvBuffer,
            /* [out] */ BSTR *modifiedRecvBuffer,
            /* [out] */ unsigned int *remaining);
        
        HRESULT ( STDMETHODCALLTYPE *SendFilter )( 
            ISslPyFilter * This,
            /* [in] */ unsigned int process,
            /* [in] */ unsigned int thread,
            /* [in] */ unsigned int socket,
            /* [in] */ BSTR *sendBuffer,
            /* [out] */ BSTR *modifiedSendBuffer);
        
        END_INTERFACE
    } ISslPyFilterVtbl;

    interface ISslPyFilter
    {
        CONST_VTBL struct ISslPyFilterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISslPyFilter_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISslPyFilter_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISslPyFilter_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISslPyFilter_EncryptMessagePrologueFilter(This,process,thread,encryptBuffer,modifiedEncryptBuffer)	\
    ( (This)->lpVtbl -> EncryptMessagePrologueFilter(This,process,thread,encryptBuffer,modifiedEncryptBuffer) ) 

#define ISslPyFilter_EncryptMessageEpilogueFilter(This,process,thread,unencryptedBuffer,encryptedBuffer)	\
    ( (This)->lpVtbl -> EncryptMessageEpilogueFilter(This,process,thread,unencryptedBuffer,encryptedBuffer) ) 

#define ISslPyFilter_DecryptMessageEpilogueFilter(This,process,thread,decryptBuffer,modifiedDecryptBuffer)	\
    ( (This)->lpVtbl -> DecryptMessageEpilogueFilter(This,process,thread,decryptBuffer,modifiedDecryptBuffer) ) 

#define ISslPyFilter_RecvFilter(This,process,thread,socket,recvBuffer,modifiedRecvBuffer,remaining)	\
    ( (This)->lpVtbl -> RecvFilter(This,process,thread,socket,recvBuffer,modifiedRecvBuffer,remaining) ) 

#define ISslPyFilter_SendFilter(This,process,thread,socket,sendBuffer,modifiedSendBuffer)	\
    ( (This)->lpVtbl -> SendFilter(This,process,thread,socket,sendBuffer,modifiedSendBuffer) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISslPyFilter_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


