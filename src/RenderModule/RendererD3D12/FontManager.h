#pragma once

class D3D12Renderer;
class FontManager
{
    D3D12Renderer       *m_pRenderer = nullptr;
    ID2D1Device2        *m_pD2DDevice = nullptr;
    ID2D1DeviceContext2 *m_pD2DDeviceContext = nullptr;

    ID2D1Bitmap1           *m_pD2DTargetBitmap = nullptr;
    ID2D1Bitmap1           *m_pD2DTargetBitmapRead = nullptr;
    IDWriteFontCollection1 *m_pFontCollection = nullptr;
    ID2D1SolidColorBrush   *m_pWhiteBrush = nullptr;

    IDWriteFactory5     *m_pDWFactory = nullptr;
    DWRITE_LINE_METRICS *m_pLineMetrics = nullptr;

    UINT m_maxLineMetricsNum = 0;
    UINT m_D2DBitmapWidth = 0;
    UINT m_D2DBitmapHeight = 0;

  private:
    BOOL CreateD2D(ID3D12Device *pD3DDevice, ID3D12CommandQueue *pCommandQueue, BOOL bEnableDebugLayer);
    BOOL CreateDWrite(ID3D12Device *pD3DDevice, UINT TexWidth, UINT TexHeight, float fDPI);
    BOOL CreateBitmapFontText(int *pOutWidth, int *pOutHeight, IDWriteTextFormat *pTextFormat, const WCHAR *inStr,
                              UINT len);
    
    void CleanupDWrite();
    void CleanupD2D();
    void Cleanup();

  public:
    BOOL Initialize(D3D12Renderer *pRenderer, ID3D12CommandQueue *pCommandQueue, UINT width, UINT height,
                    BOOL bEnableDebugLayer);
    FONT_HANDLE *CreateFontObject(const WCHAR *fontFamilyName, float fontSize);
    void         DeleteFontObject(FONT_HANDLE *pFontHandle);
    BOOL         WriteTextToBitmap(BYTE *pDestImage, UINT destWidth, UINT destHeight, UINT destPitch, int *pOutWidth,
                                   int *pOutHeight, FONT_HANDLE *pFontHandle, const WCHAR *inStr, UINT len);
    FontManager() = default;
    ~FontManager();

};