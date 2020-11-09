#include "stdafx.h"
#include "Direct2DImageCtrl.h"
#include <algorithm>

#define DIRECT2D_EFFECT_FILES_FOLDER             _T("Effect Resources")

#define PARAMETER_SECTION                        _T("EffectParameter")
#define PARAMETER_NAME                           _T("IDS_EFFECT_NAME")
#define PARAMETER_GUID                           _T("IDS_EFFECT_GUID")
#define PARAMETER_LUT_IMAGE                      _T("IDS_EFFECT_LUT_IMAGE")

#define GOLDEN_GUID                              _T("FBBDD398-21AB-4005-9A7F-5C41287049D8")
#define NOIR_GUID                                _T("5CB857BE-5E9F-43EC-B39B-46EA7A0579CE")

#define MAX_LENGTH  512

IMPLEMENT_DYNAMIC(CDirect2DImageCtrl, CStatic)

CDirect2DImageCtrl::CDirect2DImageCtrl()
{
   m_customEffect = nullptr;

   m_bUseGoldenEffect = FALSE;
   m_bUseNoirEffect = FALSE;
}

CDirect2DImageCtrl::~CDirect2DImageCtrl()
{

}

BEGIN_MESSAGE_MAP(CDirect2DImageCtrl, CStatic)
   ON_WM_SIZE()
   ON_WM_PAINT()
END_MESSAGE_MAP()

void CDirect2DImageCtrl::PreSubclassWindow()
{
   LoadDirect2DEffect();

   CreateDeviceIndependentResources();

   CStatic::PreSubclassWindow();
}

void CDirect2DImageCtrl::SetEffect(BOOL bGolden, BOOL bNoir)
{
   m_bUseGoldenEffect = bGolden;
   m_bUseNoirEffect = bNoir;

   UpdateEffect();
}

void CDirect2DImageCtrl::LoadDirect2DEffect()
{
   TCHAR szAppPath[MAX_PATH + 1];

   GetModuleFileName(NULL, szAppPath, sizeof(szAppPath) / sizeof(TCHAR));

   CString strEffectPath = szAppPath;

   if (strEffectPath.IsEmpty())
   {
      return;
   }

   int nPos = strEffectPath.ReverseFind('\\');
   strEffectPath = strEffectPath.Left(nPos);

   nPos= strEffectPath.ReverseFind('\\');
   strEffectPath = strEffectPath.Left(nPos);
   
   nPos = strEffectPath.ReverseFind('\\');
   strEffectPath = strEffectPath.Left(nPos);

   strEffectPath += _T("\\");
   strEffectPath += DIRECT2D_EFFECT_FILES_FOLDER;

   if (!PathIsDirectory(strEffectPath))
   {
      return;
   }

   strEffectPath += _T("\\*.ini");

   CFileFind Finder;
   BOOL bFound = Finder.FindFile(strEffectPath);
   while (bFound)
   {
      bFound = Finder.FindNextFileW();
      CString strFileName = Finder.GetFilePath();

      LoadDirect2DEffectFromFile(strFileName);
   }
   Finder.Close();
}

void CDirect2DImageCtrl::LoadDirect2DEffectFromFile(CString strFilePath)
{
   Direct2DCustomEffect struEffect;

   CString strName = GetParamterValue(PARAMETER_SECTION, PARAMETER_NAME, strFilePath);

   if (strName.IsEmpty())
   {
      return;
   }

   struEffect.strName = strName;

   CString strEffectGUID = GetParamterValue(PARAMETER_SECTION, PARAMETER_GUID, strFilePath);

   if (strEffectGUID.IsEmpty())
   {
      return;
   }

   RPC_STATUS status = UuidFromString((RPC_WSTR)(LPCTSTR)strEffectGUID, &struEffect.guidEffect);

   if (status != RPC_S_OK)
   {
      return;
   }

   int nPos = strFilePath.ReverseFind('\\');
   CString strFolder = strFilePath.Left(nPos);
   strFolder += _T("\\");

   CString strLUTImage = GetParamterValue(PARAMETER_SECTION, PARAMETER_LUT_IMAGE, strFilePath);

   if (strLUTImage.IsEmpty())
   {
      return;
   }

   struEffect.strLUTImageFilePath = strFolder + strLUTImage;

   m_vecCustomEffect.push_back(struEffect);
}

void CDirect2DImageCtrl::CreateDeviceIndependentResources()
{
   HRESULT hr = S_OK;

   IDXGIAdapter *pDxgiAdapter = nullptr;
   ID3D11Device *pD3D11Device = nullptr;
   ID3D11DeviceContext *pD3D11DeviceContext = nullptr;
   IDXGIDevice1 *pDxgiDevice = nullptr;
   IDXGIFactory2 *pDxgiFactory = nullptr;
   IDXGISurface *pDxgiBackBuffer = nullptr;

   UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
   creationFlags |= D3D11_CREATE_DEVICE_DEBUG;

   D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0,
      D3D_FEATURE_LEVEL_9_3,
      D3D_FEATURE_LEVEL_9_2,
      D3D_FEATURE_LEVEL_9_1
   };

   D3D_FEATURE_LEVEL featureLevel;

   hr = D3D11CreateDevice(pDxgiAdapter, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, featureLevels, sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION, &pD3D11Device, &featureLevel, &pD3D11DeviceContext);

   if (SUCCEEDED(hr))
   {
      hr = pD3D11Device->QueryInterface(__uuidof(IDXGIDevice1), (void **)&pDxgiDevice);
   }

   if (SUCCEEDED(hr))
   {
      hr = pDxgiDevice->GetAdapter(&pDxgiAdapter);
   }

   if (SUCCEEDED(hr))
   {
      hr = pDxgiAdapter->GetParent(IID_PPV_ARGS(&pDxgiFactory));
   }

   if (SUCCEEDED(hr))
   {
      DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc;
      fullscreenDesc.RefreshRate.Numerator = 60;
      fullscreenDesc.RefreshRate.Denominator = 1;
      fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
      fullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
      fullscreenDesc.Windowed = FALSE;

      DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
      swapChainDesc.Height = 0;
      swapChainDesc.Width = 0;
      swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
      swapChainDesc.BufferCount = 2;
      swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      swapChainDesc.Flags = 0;
      swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
      swapChainDesc.SampleDesc.Count = 1;
      swapChainDesc.SampleDesc.Quality = 0;
      swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
      swapChainDesc.Stereo = FALSE;
      swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

      hr = pDxgiFactory->CreateSwapChainForHwnd(pD3D11Device, m_hWnd, &swapChainDesc, nullptr, nullptr, &m_pSwapChain);
   }

   if (SUCCEEDED(hr))
   {
      hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pDxgiBackBuffer));
   }

   if (SUCCEEDED(hr))
   {
      hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &m_pD2DFactory);
   }

   if (SUCCEEDED(hr))
   {
      hr = m_pD2DFactory->CreateDevice(pDxgiDevice, &m_pDevice);
   }

   if (SUCCEEDED(hr))
   {
      hr = m_pDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pD2DContext);
      m_pD2DContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
   }

   if (SUCCEEDED(hr))
   {
      D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
         D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
         D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
         96.0f,
         96.0f
      );

      hr = m_pD2DContext->CreateBitmapFromDxgiSurface(pDxgiBackBuffer, &bitmapProperties, &m_pTargetBitmap);
   }

   CoInitialize(nullptr);

   if (SUCCEEDED(hr))
   {
      hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_wicFactory));
   }

   if (SUCCEEDED(hr))
   {
      hr = m_wicFactory->CreateDecoderFromFilename(
         L"TransitionSource.png",
         nullptr,
         GENERIC_READ,
         WICDecodeMetadataCacheOnDemand,
         &m_wicBitmapDecoder);
   }

   if (SUCCEEDED(hr))
   {
      hr = m_wicBitmapDecoder->GetFrame(0, &m_wicBitmapFrameDecoder);
   }

   WICPixelFormatGUID pPixelFormat;

   if (SUCCEEDED(hr))
   {
      hr = m_wicBitmapFrameDecoder->GetPixelFormat(&pPixelFormat);

      hr = m_wicFactory->CreateFormatConverter(&m_wicFormatConverter);
   }

   if (SUCCEEDED(hr))
   {
      hr = m_wicFormatConverter->Initialize(
         m_wicBitmapFrameDecoder.Get(),
         GUID_WICPixelFormat32bppPBGRA,
         WICBitmapDitherTypeNone,
         nullptr,
         0.0f,
         WICBitmapPaletteTypeCustom
      );
   }

   UINT width;
   UINT height;

   if (SUCCEEDED(hr))
   {
      hr = m_pD2DContext->CreateBitmapFromWicBitmap(m_wicFormatConverter.Get(), NULL, &m_pD2DImage);

      hr = m_wicFormatConverter->GetSize(&width, &height);

      m_uszImage = D2D1::SizeU(width, height);
   }

   // LUT
   for (int i = 0; i < m_vecCustomEffect.size(); i++)
   {
      hr = m_wicFactory->CreateDecoderFromFilename(
        m_vecCustomEffect[i].strLUTImageFilePath,
         nullptr,
         GENERIC_READ,
         WICDecodeMetadataCacheOnDemand,
         &m_wicLUTBitmapDecoder);

      if (SUCCEEDED(hr))
      {
         hr = m_wicLUTBitmapDecoder->GetFrame(0, &m_wicLUTBitmapFrameDecoder);
      }

      if (SUCCEEDED(hr))
      {
         hr = m_wicLUTBitmapFrameDecoder->GetPixelFormat(&pPixelFormat);

         hr = m_wicFactory->CreateFormatConverter(&m_wicLUTFormatConverter);
      }

      if (SUCCEEDED(hr))
      {
         hr = m_wicLUTFormatConverter->Initialize(
            m_wicLUTBitmapFrameDecoder.Get(),
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0f,
            WICBitmapPaletteTypeCustom
         );
      }

      if (SUCCEEDED(hr))
      {
         hr = m_pD2DContext->CreateBitmapFromWicBitmap(m_wicLUTFormatConverter.Get(), NULL, &m_vecCustomEffect[i].pD2DLUTImage);

         hr = m_wicLUTFormatConverter->GetSize(&width, &height);

         m_uszLUT = D2D1::SizeU(width, height);
      }
   }

   CoUninitialize();
}

CString CDirect2DImageCtrl::GetParamterValue(CString strSection, CString strLable, CString strFilePath)
{
   CString strFound;

   GetPrivateProfileString(strSection, strLable, _T(""), strFound.GetBuffer(MAX_LENGTH), MAX_LENGTH, strFilePath);
   strFound.ReleaseBuffer();

   return strFound;
}

void CDirect2DImageCtrl::UpdateEffect()
{
   if (!m_bUseGoldenEffect && !m_bUseNoirEffect)
   {
      m_customEffect = nullptr;
   }
   else if (m_bUseGoldenEffect && m_bUseNoirEffect)
   {
      // create golden
      HRESULT hr = CustomEffect::Register(m_pD2DFactory.Get());

      ComPtr<ID2D1Effect> pGoldenEffect = nullptr;

      if (SUCCEEDED(hr))
      {
         hr = m_pD2DContext->CreateEffect(CLSID_CustomEffect, &pGoldenEffect);
      }

      if (SUCCEEDED(hr))
      {
         GUID guidGolden;
         UuidFromString((RPC_WSTR)(LPCTSTR)GOLDEN_GUID, &guidGolden);
         
         auto pEffect = std::find_if(m_vecCustomEffect.begin(), m_vecCustomEffect.end(), [guidGolden](Direct2DCustomEffect& effectItem) {return effectItem.guidEffect == guidGolden; });
         if (pEffect != m_vecCustomEffect.end())
         {
            UINT32 nInputCount = pGoldenEffect->GetInputCount();

            pGoldenEffect->SetInput(0, m_pD2DImage.Get());
            pGoldenEffect->SetInput(1, pEffect->pD2DLUTImage.Get());
         }
      }

      if (SUCCEEDED(hr))
      {
         hr = m_pD2DContext->CreateEffect(CLSID_CustomEffect, &m_customEffect);
      }

      if (SUCCEEDED(hr))
      {
         GUID noirGolden;
         UuidFromString((RPC_WSTR)(LPCTSTR)NOIR_GUID, &noirGolden);

         auto pEffect = std::find_if(m_vecCustomEffect.begin(), m_vecCustomEffect.end(), [noirGolden](Direct2DCustomEffect& effectItem) {return effectItem.guidEffect == noirGolden; });
         if (pEffect != m_vecCustomEffect.end())
         {
            UINT32 nInputCount = m_customEffect->GetInputCount();

            m_customEffect->SetInputEffect(0, pGoldenEffect.Get());
            m_customEffect->SetInput(1, pEffect->pD2DLUTImage.Get());
         }
      }
   }
   else if (m_bUseGoldenEffect)
   {
      HRESULT hr = CustomEffect::Register(m_pD2DFactory.Get());

      if (SUCCEEDED(hr))
      {
         hr = m_pD2DContext->CreateEffect(CLSID_CustomEffect, &m_customEffect);
      }

      if (SUCCEEDED(hr))
      {
         GUID guidGolden;
         UuidFromString((RPC_WSTR)(LPCTSTR)GOLDEN_GUID, &guidGolden);

         auto pEffect = std::find_if(m_vecCustomEffect.begin(), m_vecCustomEffect.end(), [guidGolden](Direct2DCustomEffect& effectItem) {return effectItem.guidEffect == guidGolden; });
         if (pEffect != m_vecCustomEffect.end())
         {
            UINT32 nInputCount = m_customEffect->GetInputCount();

            m_customEffect->SetInput(0, m_pD2DImage.Get());
            m_customEffect->SetInput(1, pEffect->pD2DLUTImage.Get());
         }
      }
   }
   else if (m_bUseNoirEffect)
   {
      HRESULT hr = CustomEffect::Register(m_pD2DFactory.Get());

      if (SUCCEEDED(hr))
      {
         hr = m_pD2DContext->CreateEffect(CLSID_CustomEffect, &m_customEffect);
      }

      if (SUCCEEDED(hr))
      {
         GUID noirGolden;
         UuidFromString((RPC_WSTR)(LPCTSTR)NOIR_GUID, &noirGolden);

         auto pEffect = std::find_if(m_vecCustomEffect.begin(), m_vecCustomEffect.end(), [noirGolden](Direct2DCustomEffect& effectItem) {return effectItem.guidEffect == noirGolden; });
         if (pEffect != m_vecCustomEffect.end())
         {
            UINT32 nInputCount = m_customEffect->GetInputCount();

            m_customEffect->SetInput(0, m_pD2DImage.Get());
            m_customEffect->SetInput(1, pEffect->pD2DLUTImage.Get());
         }
      }
   }

   Invalidate();
   UpdateWindow();
}

void CDirect2DImageCtrl::OnSize(UINT nType, int cx, int cy)
{
   CStatic::OnSize(nType, cx, cy);

   if (m_pSwapChain == nullptr || m_pD2DContext == nullptr)
   {
      return;
   }

   CRect rcCient;
   GetClientRect(rcCient);

   if (rcCient.IsRectEmpty())
   {
      return;
   }

   ID2D1Image* pImage = nullptr;
   m_pD2DContext->GetTarget(&pImage);
   m_pD2DContext->SetTarget(nullptr);
   if (pImage)
   {
      pImage->Release();
   }

   m_pTargetBitmap = nullptr;

   IDXGISurface* pBuffer = NULL;
   m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBuffer));
   if (pBuffer)
   {
      int i = pBuffer->Release();
      while (i > 0)
         i = pBuffer->Release();
   }
   pBuffer = NULL;
   m_pSwapChain->GetBuffer(1, IID_PPV_ARGS(&pBuffer));
   if (pBuffer)
   {
      int i = pBuffer->Release();
      while (i > 0)
         i = pBuffer->Release();
   }

   HRESULT hr = m_pSwapChain->ResizeBuffers(0, rcCient.Width(), rcCient.Height(), DXGI_FORMAT_UNKNOWN, 0);

   if (FAILED(hr))
   {
      return;
   }

   FLOAT dpiX = 96.0;
   FLOAT dpiY = 96.0;
   // Direct2D needs the dxgi version of the backbuffer surface pointer.
   ComPtr<IDXGISurface> dxgiBackBuffer;
   hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
   if (hr != S_OK)
   {
      return;
   }
   // Create a Direct2D surface (bitmap) linked to the Direct3D texture back buffer via the DXGI back buffer
   //DON'T CHANGE THESE VALUES!!! Bitmap creation will fail with anything but these values.
   D2D1_BITMAP_PROPERTIES1 bitmapProperties =
      D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
         D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), dpiX, dpiY);

   // Get a D2D surface from the DXGI back buffer to use as the D2D render target.

   hr = m_pD2DContext->CreateBitmapFromDxgiSurface(
      dxgiBackBuffer.Get(),
      &bitmapProperties,
      &m_pTargetBitmap);

   if (hr != S_OK)
   {
      return;
   }
}

void CDirect2DImageCtrl::OnPaint()
{
   CPaintDC dc(this);

   CRect rcClient;
   GetClientRect(rcClient);

   D2D1_SIZE_F rectTargrt = m_pTargetBitmap->GetSize();

   m_pD2DContext->BeginDraw();
   m_pD2DContext->SetTarget(m_pTargetBitmap.Get());

   m_pD2DContext->Clear(D2D1::ColorF(D2D1::ColorF::Black/*CornflowerBlue*/));

   if (m_customEffect != nullptr)
   {
      m_pD2DContext->DrawImage(m_customEffect.Get());
   }
   else
   {
      D2D1_RECT_F rcDst = D2D1::RectF(0.F, 0.F, (FLOAT)rcClient.Width(), (FLOAT)rcClient.Height());

      m_pD2DContext->DrawBitmap(m_pD2DImage.Get(), rcDst);
   }

   // We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
   // is lost. It will be handled during the next call to Present.
   HRESULT hr = m_pD2DContext->EndDraw();
   DXGI_PRESENT_PARAMETERS parameters = { 0 };
   parameters.DirtyRectsCount = 0;
   parameters.pDirtyRects = nullptr;
   parameters.pScrollRect = nullptr;
   parameters.pScrollOffset = nullptr;

   hr = m_pSwapChain->Present1(1, 0, &parameters);

}