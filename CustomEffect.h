#pragma once

#include <d2d1.h>
#include <wrl\client.h>
#include <d2d1_1.h>
#include <d2d1_2.h>
#include <d3d11.h>
#include <d3d11_2.h>
#include <d2d1_1helper.h>
#include <d2d1effectauthor.h>
#include <d2d1effecthelpers.h>
#include <math.h>
#include <initguid.h>

using namespace Microsoft::WRL;

DEFINE_GUID(GUID_CustomPixelShader, 0xf1fb0c6f, 0x6d53, 0x4ee5, 0x9f, 0x17, 0xae, 0x3f, 0xb, 0xf1, 0xfd, 0x53);// {F1FB0C6F-6D53-4EE5-9F17-AE3F0BF1FD53}
DEFINE_GUID(CLSID_CustomEffect, 0xfbbdd398, 0x21ab, 0x4005, 0x9a, 0x7f, 0x5c, 0x41, 0x28, 0x70, 0x49, 0xd8);// {FBBDD398-21AB-4005-9A7F-5C41287049D8}

class CustomEffect : public ID2D1EffectImpl, public ID2D1DrawTransform
{
public:
   // Declare effect registration methods.
   static HRESULT Register(_In_ ID2D1Factory1* pFactory);

   static HRESULT __stdcall CreateCustomImpl(_Outptr_ IUnknown** ppEffectImpl);

   IFACEMETHODIMP Initialize(
      _In_ ID2D1EffectContext* pContextInternal,
      _In_ ID2D1TransformGraph* pTransformGraph
   );

   IFACEMETHODIMP PrepareForRender(D2D1_CHANGE_TYPE changeType);

   IFACEMETHODIMP SetGraph(_In_ ID2D1TransformGraph* pGraph);

   IFACEMETHODIMP SetDrawInfo(_In_ ID2D1DrawInfo* pRenderInfo);

   IFACEMETHODIMP MapOutputRectToInputRects(
      _In_ const D2D1_RECT_L* pOutputRect,
      _Out_writes_(inputRectCount) D2D1_RECT_L* pInputRects,
      UINT32 inputRectCount
   ) const;

   IFACEMETHODIMP MapInputRectsToOutputRect(
      _In_reads_(inputRectCount) CONST D2D1_RECT_L* pInputRects,
      _In_reads_(inputRectCount) CONST D2D1_RECT_L* pInputOpaqueSubRects,
      UINT32 inputRectCount,
      _Out_ D2D1_RECT_L* pOutputRect,
      _Out_ D2D1_RECT_L* pOutputOpaqueSubRect
   );

   IFACEMETHODIMP MapInvalidRect(
      UINT32 inputIndex,
      D2D1_RECT_L invalidInputRect,
      _Out_ D2D1_RECT_L* pInvalidOutputRect
   ) const;

   // Declare ID2D1TransformNode implementation methods.
   IFACEMETHODIMP_(UINT32) GetInputCount() const;

   // Declare IUnknown implementation methods.
   IFACEMETHODIMP_(ULONG) AddRef();
   IFACEMETHODIMP_(ULONG) Release();
   IFACEMETHODIMP QueryInterface(_In_ REFIID riid, _Outptr_ void** ppOutput);

private:
   CustomEffect();
   HRESULT UpdateConstants();

   Microsoft::WRL::ComPtr<ID2D1DrawInfo>      m_drawInfo;
   Microsoft::WRL::ComPtr<ID2D1EffectContext> m_effectContext;
   LONG                                       m_refCount;
   D2D1_RECT_L                                m_inputRect;
   D2D1_RECT_L                                m_inputLUTRect;
   float                                      m_dpi;
   Microsoft::WRL::ComPtr<ID2D1TransformGraph>  m_transformGraph;
};
