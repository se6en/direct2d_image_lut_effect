#include "stdafx.h"
#include "CustomEffect.h"
#include <vector>

#define XML(X) TEXT(#X)

void SplitPathW(LPCWSTR pszSrcPath, LPWSTR head, LPWSTR tail, int iSplit)
{
   *head = *tail = L'\0';

   LPCWSTR pszSplit;
   if (iSplit == -1)
   {
      if (wcsncmp(pszSrcPath, L"\\\\", 2) == 0)
         pszSplit = wcschr(pszSrcPath + 2, L'\\'); // ignore the '\\' portion of UNC paths
      else
         pszSplit = wcschr(pszSrcPath, L'\\');
   }
   else
      pszSplit = wcsrchr(pszSrcPath, L'\\');

   if (pszSplit)
   {
      int len = (int)(pszSplit - pszSrcPath);
      wcsncpy(head, pszSrcPath, len);
      head[len] = L'\0';
      wcscpy(tail, pszSplit + 1);
   }
   else
   {
      if (iSplit == -1)
         wcscpy(head, pszSrcPath);
      else
         wcscpy(tail, pszSrcPath);
   }
}

BOOL LoadEffectFromFile(CString strFileName, BYTE** ppData, DWORD* pNumBytes)
{
   HANDLE hFile = NULL;
   BOOL fSuccess = TRUE;

   if (strFileName.GetLength() < 1)
   {
      throw(-1);
   }

   hFile = CreateFile(strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
   if (hFile == INVALID_HANDLE_VALUE)
   {
      return FALSE;
   }

   DWORD dwByteSizeOfFile = GetFileSize(hFile, NULL);
   if (dwByteSizeOfFile == INVALID_FILE_SIZE)
   {
      OutputDebugString(_T("    GetFileSize() FAILED in LoadEffectFromFile()\n"));
      return FALSE;
   }

   *ppData = new BYTE[dwByteSizeOfFile];
   if (!ppData)
   {
      OutputDebugString(_T("    allocation FAILED in LoadEffectFromFile()\n"));
      return FALSE;
   }

   if (!ReadFile(hFile, *ppData, dwByteSizeOfFile, pNumBytes, NULL))
   {
      OutputDebugString(_T("    ReadFile FAILED in LoadEffectFromFile()\n"));
      return FALSE;
   }

   CloseHandle(hFile);

   return TRUE;
}

CustomEffect::CustomEffect() :
   m_refCount(1)
{

}

HRESULT __stdcall CustomEffect::CreateCustomImpl(_Outptr_ IUnknown** ppEffectImpl)
{
   // Since the object's refcount is initialized to 1, we don't need to AddRef here.
   *ppEffectImpl = static_cast<ID2D1EffectImpl*>(new (std::nothrow) CustomEffect());

   if (*ppEffectImpl == nullptr)
   {
      return E_OUTOFMEMORY;
   }
   else
   {
      return S_OK;
   }
}

HRESULT CustomEffect::Register(_In_ ID2D1Factory1* pFactory)
{
   // The inspectable metadata of an effect is defined in XML. This can be passed in from an external source
   // as well, however for simplicity we just inline the XML.
   PCWSTR pszXml =
      XML(
         <?xml version = '1.0'?>
         <Effect>
         <!--System Properties-->
         <Property name = 'DisplayName' type = 'string' value = 'Custom'/>
         <Property name = 'Author' type = 'string' value = 'Dexter'/>
         <Property name = 'Category' type = 'string' value = 'Stylize'/>
         <Property name = 'Description' type = 'string' value = 'Adds a custom effect that can be animated'/>
         <Inputs>
         <Input name = 'Source'/>
         <Input name = 'LUTTexture'/>
         </Inputs>
         <!--Custom Properties go here-->
         </Effect>
      );

   // This registers the effect with the factory, which will make the effect
   // instantiatable.
   HRESULT hr = pFactory->RegisterEffectFromString(
      CLSID_CustomEffect,
      pszXml,
      NULL,
      0,
      CreateCustomImpl
   );

   return hr;
}

IFACEMETHODIMP CustomEffect::Initialize(
   _In_ ID2D1EffectContext* pEffectContext,
   _In_ ID2D1TransformGraph* pTransformGraph
)
{
   // To maintain consistency across different DPIs, this effect needs to cover more pixels at
   // higher than normal DPIs. The context is saved here so the effect can later retrieve the DPI.
   m_effectContext = pEffectContext;
   m_transformGraph = pTransformGraph;

   TCHAR szPath[MAX_PATH + 1];
   TCHAR szAppPath[MAX_PATH + 1];
   TCHAR szFileName[MAX_PATH + 1];

   GetModuleFileName(NULL, szAppPath, sizeof(szAppPath) / sizeof(TCHAR));
   SplitPathW(szAppPath, szPath, szFileName, 0);
   PathAppend(szPath, _T("CustomEffect.cso"));

   BYTE* pData = NULL;
   DWORD dwBytes = 0;
   if (!LoadEffectFromFile(szPath, &pData, &dwBytes))
   {
      return 1;
   }
   _ASSERT(pData);

   HRESULT hr = pEffectContext->LoadPixelShader(GUID_CustomPixelShader, pData, dwBytes);
   delete[] pData;

   // This loads the shader into the Direct2D image effects system and associates it with the GUID passed in.
   // If this method is called more than once (say by other instances of the effect) with the same GUID,
   // the system will simply do nothing, ensuring that only one instance of a shader is stored regardless of how
   // many time it is used.
   if (SUCCEEDED(hr))
   {
      // The graph consists of a single transform. In fact, this class is the transform,
      // reducing the complexity of implementing an effect when all we need to
      // do is use a single pixel shader.
      hr = pTransformGraph->SetSingleTransformNode(this);
   }

   return hr;
}

HRESULT CustomEffect::UpdateConstants()
{
   // Update the DPI if it has changed. This allows the effect to scale across different DPIs automatically.
   m_effectContext->GetDpi(&m_dpi, &m_dpi);

   return S_OK;
}

IFACEMETHODIMP CustomEffect::PrepareForRender(D2D1_CHANGE_TYPE changeType)
{
   SetGraph(m_transformGraph.Get());

   return S_OK;
}

// SetGraph is only called when the number of inputs changes. This never happens as we publish this effect
// as a single input effect.
IFACEMETHODIMP CustomEffect::SetGraph(_In_ ID2D1TransformGraph* pGraph)
{
   HRESULT hr = S_OK;

   hr = pGraph->AddNode(this);
   hr = pGraph->ConnectToEffectInput(0, this, 0);
   hr = pGraph->ConnectToEffectInput(1, this, 1);
   hr = pGraph->SetOutputNode(this);

   return hr;
}

// Called to assign a new render info class, which is used to inform D2D on
// how to set the state of the GPU.
IFACEMETHODIMP CustomEffect::SetDrawInfo(_In_ ID2D1DrawInfo* pDrawInfo)
{
   HRESULT hr = S_OK;

   hr = pDrawInfo->SetPixelShader(GUID_CustomPixelShader);

   m_drawInfo = pDrawInfo;
   UpdateConstants();

   return hr;
}

// Calculates the mapping between the output and input rects. In this case,
// we want to request an expanded region to account for pixels that the ripple
// may need outside of the bounds of the destination.
IFACEMETHODIMP CustomEffect::MapOutputRectToInputRects(
   _In_ const D2D1_RECT_L* pOutputRect,
   _Out_writes_(inputRectCount) D2D1_RECT_L* pInputRects,
   UINT32 inputRectCount
) const
{
   // This effect has exactly one input, so if there is more than one input rect,
   // something is wrong.
   if (inputRectCount != 2)
   {
      return E_INVALIDARG;
   }

   pInputRects[0] = *pOutputRect;
   pInputRects[1] = m_inputLUTRect;

   return S_OK;
}

IFACEMETHODIMP CustomEffect::MapInputRectsToOutputRect(
   _In_reads_(inputRectCount) CONST D2D1_RECT_L* pInputRects,
   _In_reads_(inputRectCount) CONST D2D1_RECT_L* pInputOpaqueSubRects,
   UINT32 inputRectCount,
   _Out_ D2D1_RECT_L* pOutputRect,
   _Out_ D2D1_RECT_L* pOutputOpaqueSubRect
)
{
   // This effect has exactly one input, so if there is more than one input rect,
   // something is wrong.
   if (inputRectCount != 2)
   {
      return E_INVALIDARG;
   }

   m_inputRect = pInputRects[0];
   m_inputLUTRect = pInputRects[1];

   *pOutputRect = pInputRects[0];

   // Indicate that entire output might contain transparency.
   ZeroMemory(pOutputOpaqueSubRect, sizeof(*pOutputOpaqueSubRect));

   return S_OK;
}

IFACEMETHODIMP CustomEffect::MapInvalidRect(
   UINT32 inputIndex,
   D2D1_RECT_L invalidInputRect,
   _Out_ D2D1_RECT_L* pInvalidOutputRect
) const
{
   HRESULT hr = S_OK;

   // Indicate that the entire output may be invalid.
   *pInvalidOutputRect = m_inputRect;

   return hr;
}

IFACEMETHODIMP_(UINT32) CustomEffect::GetInputCount() const
{
   return 2;
}

// D2D ensures that that effects are only referenced from one thread at a time.
// To improve performance, we simply increment/decrement our reference count
// rather than use atomic InterlockedIncrement()/InterlockedDecrement() functions.
IFACEMETHODIMP_(ULONG) CustomEffect::AddRef()
{
   m_refCount++;
   return m_refCount;
}

IFACEMETHODIMP_(ULONG) CustomEffect::Release()
{
   m_refCount--;

   if (m_refCount == 0)
   {
      delete this;
      return 0;
   }
   else
   {
      return m_refCount;
   }
}

// This enables the stack of parent interfaces to be queried. In the instance
// of the Ripple interface, this method simply enables the developer
// to cast a Ripple instance to an ID2D1EffectImpl or IUnknown instance.
IFACEMETHODIMP CustomEffect::QueryInterface(
   _In_ REFIID riid,
   _Outptr_ void** ppOutput
)
{
   *ppOutput = nullptr;
   HRESULT hr = S_OK;

   if (riid == __uuidof(ID2D1EffectImpl))
   {
      *ppOutput = reinterpret_cast<ID2D1EffectImpl*>(this);
   }
   else if (riid == __uuidof(ID2D1DrawTransform))
   {
      *ppOutput = static_cast<ID2D1DrawTransform*>(this);
   }
   else if (riid == __uuidof(ID2D1Transform))
   {
      *ppOutput = static_cast<ID2D1Transform*>(this);
   }
   else if (riid == __uuidof(ID2D1TransformNode))
   {
      *ppOutput = static_cast<ID2D1TransformNode*>(this);
   }
   else if (riid == __uuidof(IUnknown))
   {
      *ppOutput = this;
   }
   else
   {
      hr = E_NOINTERFACE;
   }

   if (*ppOutput != nullptr)
   {
      AddRef();
   }

   return hr;
}