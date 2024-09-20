#include "pch.h"
#include "D3D12Renderer.h"
#include "FontManager.h"

BOOL FontManager::CreateD2D(ID3D12Device *pD3DDevice, ID3D12CommandQueue *pCommandQueue, BOOL bEnableDebugLayer)
{
    UINT                 d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};

    ID2D1Factory3       *pD2DFactory = nullptr;
    ID3D11Device        *pD3D11Device = nullptr;
    ID3D11DeviceContext *pD3D11DeviceContext = nullptr;
    ID3D11On12Device    *pD3D11On12Device = nullptr;
    if (bEnableDebugLayer)
    {
        d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
    }

    if (FAILED(D3D11On12CreateDevice(pD3DDevice, d3d11DeviceFlags, nullptr, 0, (IUnknown **)&pCommandQueue, 1, 0,
                                     &pD3D11Device, &pD3D11DeviceContext, nullptr)))
    {
        __debugbreak();
    }
    if (FAILED(pD3D11Device->QueryInterface(IID_PPV_ARGS(&pD3D11On12Device))))
    {
        __debugbreak();
    }

    // Create D2D/DWrite Components
    D2D1_DEVICE_CONTEXT_OPTIONS deviceOption = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions,
                                 (void **)&pD2DFactory)))
    {
        __debugbreak();
    }

    IDXGIDevice *pDXGIDevice = nullptr;
    if (FAILED(pD3D11On12Device->QueryInterface(IID_PPV_ARGS(&pDXGIDevice))))
    {
        __debugbreak();
    }
    if (FAILED(pD2DFactory->CreateDevice(pDXGIDevice, &m_pD2DDevice)))
    {
        __debugbreak();
    }

    if (FAILED(m_pD2DDevice->CreateDeviceContext(deviceOption, &m_pD2DDeviceContext)))
    {
        __debugbreak();
    }

    if (pD3D11Device)
    {
        pD3D11Device->Release();
        pD3D11Device = nullptr;
    }
    if (pDXGIDevice)
    {
        pDXGIDevice->Release();
        pDXGIDevice = nullptr;
    }
    if (pD2DFactory)
    {
        pD2DFactory->Release();
        pD2DFactory = nullptr;
    }
    if (pD3D11On12Device)
    {
        pD3D11On12Device->Release();
        pD3D11On12Device = nullptr;
    }
    if (pD3D11DeviceContext)
    {
        pD3D11DeviceContext->Release();
        pD3D11DeviceContext = nullptr;
    }
    return TRUE;
}

BOOL FontManager::CreateDWrite(ID3D12Device *pD3DDevice, UINT TexWidth, UINT TexHeight, float fDPI)
{
    BOOL bResult = FALSE;

    m_D2DBitmapWidth = TexWidth;
    m_D2DBitmapHeight = TexHeight;

    // InitCustomFont(pCustomFontList, dwCustomFontNum);

    D2D1_SIZE_U size;
    size.width = TexWidth;
    size.height = TexHeight;
    
    D2D1_BITMAP_PROPERTIES1 bitmapProperties =
        D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                          D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), fDPI, fDPI);

    if (FAILED(m_pD2DDeviceContext->CreateBitmap(size, nullptr, 0, &bitmapProperties, &m_pD2DTargetBitmap)))
        __debugbreak();

    bitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_CANNOT_DRAW | D2D1_BITMAP_OPTIONS_CPU_READ;
    if (FAILED(m_pD2DDeviceContext->CreateBitmap(size, nullptr, 0, &bitmapProperties, &m_pD2DTargetBitmapRead)))
        __debugbreak();

    if (FAILED(m_pD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_pWhiteBrush)))
        __debugbreak();

    HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory5), (IUnknown **)&m_pDWFactory);
    if (FAILED(hr))
        __debugbreak();

    bResult = TRUE;
lb_return:
    return bResult;
}

BOOL FontManager::CreateBitmapFontText(int *pOutWidth, int *pOutHeight, IDWriteTextFormat *pTextFormat,
                                       const WCHAR *inStr, UINT len)
{
    BOOL result = FALSE;

    ID2D1DeviceContext *pD2DDeviceContext = m_pD2DDeviceContext;
    IDWriteFactory5    *pDWFactory = m_pDWFactory;
    D2D1_SIZE_F         max_size = pD2DDeviceContext->GetSize();
    max_size.width = (float)m_D2DBitmapWidth;
    max_size.height = (float)m_D2DBitmapHeight;

    IDWriteTextLayout *pTextLayout = nullptr;
    if (pDWFactory && pTextFormat)
    {
        if (FAILED(pDWFactory->CreateTextLayout(inStr, len, pTextFormat, max_size.width, max_size.height,
                                                &pTextLayout)))
            __debugbreak();
    }
    DWRITE_TEXT_METRICS metrics = {};
    if (pTextLayout)
    {
        pTextLayout->GetMetrics(&metrics);

        // 타겟 설정
        pD2DDeviceContext->SetTarget(m_pD2DTargetBitmap);

        // 안티앨리어싱모드 설정
        pD2DDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

        // 텍스트 렌더링
        pD2DDeviceContext->BeginDraw();

        pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::Black));
        pD2DDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

        pD2DDeviceContext->DrawTextLayout(D2D1::Point2F(0.0f, 0.0f), pTextLayout, m_pWhiteBrush);

        // We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
        // is lost. It will be handled during the next call to Present.
        pD2DDeviceContext->EndDraw();

        // 안티앨리어싱 모드 복구
        pD2DDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_DEFAULT);
        pD2DDeviceContext->SetTarget(nullptr);

        // 레이아웃 오브젝트 필요없으니 Release
        pTextLayout->Release();
        pTextLayout = nullptr;
    }
    int width = (int)ceil(metrics.width);
    int height = (int)ceil(metrics.height);

    D2D1_POINT_2U destPos = {};
    D2D1_RECT_U   srcRect = {0, 0, width, height};

    if (FAILED(m_pD2DTargetBitmapRead->CopyFromBitmap(&destPos, m_pD2DTargetBitmap, &srcRect)))
        __debugbreak();

    *pOutWidth = width;
    *pOutHeight = height;

    result = TRUE;

lb_return:
    return result;
}

void FontManager::CleanupDWrite()
{
    if (m_pD2DTargetBitmap)
    {
        m_pD2DTargetBitmap->Release();
        m_pD2DTargetBitmap = nullptr;
    }
    if (m_pD2DTargetBitmapRead)
    {
        m_pD2DTargetBitmapRead->Release();
        m_pD2DTargetBitmapRead = nullptr;
    }
    if (m_pWhiteBrush)
    {
        m_pWhiteBrush->Release();
        m_pWhiteBrush = nullptr;
    }
    if (m_pDWFactory)
    {
        m_pDWFactory->Release();
        m_pDWFactory = nullptr;
    }
}

void FontManager::CleanupD2D()
{
    if (m_pD2DDeviceContext)
    {
        m_pD2DDeviceContext->Release();
        m_pD2DDeviceContext = nullptr;
    }
    if (m_pD2DDevice)
    {
        m_pD2DDevice->Release();
        m_pD2DDevice = nullptr;
    }
}

void FontManager::Cleanup()
{
    CleanupDWrite();
    CleanupD2D();
}

BOOL FontManager::Initialize(D3D12Renderer *pRenderer, ID3D12CommandQueue *pCommandQueue, UINT width, UINT height,
                             BOOL bEnableDebugLayer)
{
    ID3D12Device *pD3DDevice = pRenderer->INL_GetD3DDevice();
    CreateD2D(pD3DDevice, pCommandQueue, bEnableDebugLayer);

    float DPI = pRenderer->GetDPI();
    CreateDWrite(pD3DDevice, width, height, DPI);

    return TRUE;
}

FONT_HANDLE *FontManager::CreateFontObject(const WCHAR *fontFamilyName, float fontSize)
{
    IDWriteTextFormat      *pTextFormat = nullptr;
    IDWriteFactory5        *pDWFactory = m_pDWFactory;
    IDWriteFontCollection1 *pFontCollection = nullptr;

    // The logical size of the font in DIP("device-independent pixel") units.A DIP equals 1 / 96 inch.

    if (pDWFactory)
    {
        if (FAILED(pDWFactory->CreateTextFormat(
                fontFamilyName,
                pFontCollection, // Font collection (nullptr sets it to use the system font collection).
                DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize,
                DEFAULT_LOCALE_NAME, &pTextFormat)))
        {
            __debugbreak();
        }
    }
    FONT_HANDLE *pFontHandle = new FONT_HANDLE;
    memset(pFontHandle, 0, sizeof(FONT_HANDLE));
    wcscpy_s(pFontHandle->fontFamilyName, fontFamilyName);
    pFontHandle->fontSize = fontSize;

    if (pTextFormat)
    {
        if (FAILED(pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING)))
            __debugbreak();

        if (FAILED(pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)))
            __debugbreak();
    }

    pFontHandle->pTextFormat = pTextFormat;

    return pFontHandle;
}

void FontManager::DeleteFontObject(FONT_HANDLE *pFontHandle)
{
    if (pFontHandle->pTextFormat)
    {
        pFontHandle->pTextFormat->Release();
        pFontHandle->pTextFormat = nullptr;
    }
    delete pFontHandle;
}

BOOL FontManager::WriteTextToBitmap(BYTE *pDestImage, UINT destWidth, UINT destHeight, UINT destPitch, int *pOutWidth,
                                    int *pOutHeight, FONT_HANDLE *pFontHandle, const WCHAR *inStr, UINT len)
{
    int textWidth = 0;
    int textHeight = 0;

    BOOL result = CreateBitmapFontText(&textWidth, &textHeight, pFontHandle->pTextFormat, inStr, len);
    if (result)
    {
        if (textWidth > (int)destWidth)
            textWidth = (int)destWidth;
        if (textHeight > (int)destHeight)
            textHeight = (int)destHeight;

        D2D1_MAPPED_RECT mappedRect;
        if (FAILED(m_pD2DTargetBitmapRead->Map(D2D1_MAP_OPTIONS_READ, &mappedRect)))
        {
            __debugbreak();
        }
        BYTE *pDest = pDestImage;
        char *pSrc = (char *)mappedRect.bits;

        for (UINT y = 0; y < (UINT)textHeight; y++)
        {
            memcpy(pDest, pSrc, textWidth * 4);
            pDest += destPitch;
            pSrc += mappedRect.pitch;
        }
        m_pD2DTargetBitmapRead->Unmap();
    }
    *pOutWidth = textWidth;
    *pOutHeight = textHeight;
    return result;
}

FontManager::~FontManager()
{
    Cleanup();
}
