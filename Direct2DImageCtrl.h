#pragma once

#include "CustomEffect.h"
#include <vector>

using namespace Microsoft::WRL;

struct Direct2DCustomEffect
{
   CString               strName;
   GUID                  guidEffect;
   CString               strLUTImageFilePath;
   ComPtr<ID2D1Bitmap1>  pD2DLUTImage;
};

class CDirect2DImageCtrl : public CStatic
{
   DECLARE_DYNAMIC(CDirect2DImageCtrl)

public:
   CDirect2DImageCtrl();
   virtual ~CDirect2DImageCtrl();

   afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg void OnPaint();

   virtual void PreSubclassWindow();

   DECLARE_MESSAGE_MAP()

public:
   void SetEffect(BOOL bGolden, BOOL bNoir);

private:
   void LoadDirect2DEffect();
   void LoadDirect2DEffectFromFile(CString strFilePath);

   void CreateDeviceIndependentResources();

   CString GetParamterValue(CString strSection, CString strLable, CString strFilePath);

   void UpdateEffect();

private:
   ComPtr<ID2D1Factory1>                     m_pD2DFactory;
   ComPtr<ID2D1Device>                       m_pDevice;
   ComPtr<ID2D1DeviceContext>                m_pD2DContext;
   ComPtr<IDXGISwapChain1>                   m_pSwapChain;
   ComPtr<ID2D1Bitmap1>                      m_pTargetBitmap;

   ComPtr<IWICImagingFactory2>               m_wicFactory;
   ComPtr<IWICBitmapDecoder>                 m_wicBitmapDecoder;
   ComPtr<IWICBitmapFrameDecode>             m_wicBitmapFrameDecoder;
   ComPtr<IWICFormatConverter>               m_wicFormatConverter;
   ComPtr<ID2D1Bitmap1>                      m_pD2DImage;

   ComPtr<IWICBitmapDecoder>                 m_wicLUTBitmapDecoder;
   ComPtr<IWICBitmapFrameDecode>             m_wicLUTBitmapFrameDecoder;
   ComPtr<IWICFormatConverter>               m_wicLUTFormatConverter;
   ComPtr<ID2D1Bitmap1>                      m_pD2DLUTImage;

   std::vector<Direct2DCustomEffect>         m_vecCustomEffect;

   ComPtr<ID2D1Effect>                       m_customEffect;

   D2D1_SIZE_U                               m_uszImage;
   D2D1_SIZE_U                               m_uszLUT;

   BOOL                                      m_bUseGoldenEffect;
   BOOL                                      m_bUseNoirEffect;
};
