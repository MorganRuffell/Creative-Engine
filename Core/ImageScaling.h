
#pragma once

class CGraphicsContext;
class CRGBBuffer;
enum DXGI_FORMAT;

namespace ImageScaling
{
    void Initialize(DXGI_FORMAT DestFormat);

    enum EScalingType { 
        kBilinear, 
        kSharpening, 
        kBicubic, 
        kLanczos, 
        kFilterCount 
    };

    void Upscale(CGraphicsContext& Context, CRGBBuffer& dest, CRGBBuffer& source, EScalingType tech = kLanczos);
}