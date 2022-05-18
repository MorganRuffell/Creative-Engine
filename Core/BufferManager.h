#pragma once

#include "CRGBBuffer.h"
#include "CGPUDepthBuffer.h"
#include "CCameraShadowBuffer.h"
#include "CGPUBuffer.h"
#include "CDirectX12Core.h"

/// <summary>
/// This class containbs all of the buffers that are inside of creative engine.
/// </summary>
namespace CGraphics
{
    // This is the scene depth buffer.
    extern CGPUDepthBuffer g_SceneDepthBuffer;  // D32_FLOAT_S8_UINT


    //These are all the RGB Colour buffers we represent all colour buffers with this CRGB buffer class.
    extern CRGBBuffer g_SceneColorBuffer;  // R11G11B10_FLOAT
    extern CRGBBuffer g_SceneNormalBuffer; // R16G16B16A16_FLOAT
    extern CRGBBuffer g_PostEffectsBuffer; // R32_UINT (to support Read-Modify-Write with a UAV)
    extern CRGBBuffer g_OverlayBuffer;     // R8G8B8A8_UNORM
    extern CRGBBuffer g_HorizontalBuffer;  // For separable (bicubic) upsampling

    //Buffer managing 3D movement
    extern CRGBBuffer g_VelocityBuffer;    // R10G10B10  (3D velocity)
    extern CCameraShadowBuffer g_ShadowBuffer;

    //Managing Screen Space Ambient Occlusion, this iss= 
    extern CRGBBuffer g_SSAOFullScreen;	    // R8_UNORM

    extern CRGBBuffer g_LinearDepth[2];	    // Normalized planar distance (0 at eye, 1 at far plane) computed from the SceneDepthBuffer
    extern CRGBBuffer g_MinMaxDepth8;		// Min and max depth values of 8x8 tiles
    extern CRGBBuffer g_MinMaxDepth16;		// Min and max depth values of 16x16 tiles
    extern CRGBBuffer g_MinMaxDepth32;		// Min and max depth values of 16x16 tiles
    extern CRGBBuffer g_DepthDownsize1;
    extern CRGBBuffer g_DepthDownsize2;
    extern CRGBBuffer g_DepthDownsize3;
    extern CRGBBuffer g_DepthDownsize4;
    extern CRGBBuffer g_DepthTiled1;
    extern CRGBBuffer g_DepthTiled2;
    extern CRGBBuffer g_DepthTiled3;
    extern CRGBBuffer g_DepthTiled4;

    // Manages non screen space ambient occlusion, remember morgan that these are created as vectors (vector drawings) before
    // they are rasterised.
    extern CRGBBuffer g_AOMerged1;
    extern CRGBBuffer g_AOMerged2;
    extern CRGBBuffer g_AOMerged3;
    extern CRGBBuffer g_AOMerged4;
    extern CRGBBuffer g_AOSmooth1;
    extern CRGBBuffer g_AOSmooth2;
    extern CRGBBuffer g_AOSmooth3;

    //Different types of Ambient Occlusion
    extern CRGBBuffer g_AOHighQuality1;
    extern CRGBBuffer g_AOHighQuality2;
    extern CRGBBuffer g_AOHighQuality3;
    extern CRGBBuffer g_AOHighQuality4;

    // External instances that manages all the Depth of field
    extern CRGBBuffer g_DoFTileClass[2];
    extern CRGBBuffer g_DoFPresortBuffer;
    extern CRGBBuffer g_DoFPrefilter;
    extern CRGBBuffer g_DoFBlurColor[2];
    extern CRGBBuffer g_DoFBlurAlpha[2];
    extern CStructuredBuffer g_DoFWorkQueue;
    extern CStructuredBuffer g_DoFFastQueue;
    extern CStructuredBuffer g_DoFFixupQueue;

    extern CRGBBuffer g_MotionPrepBuffer;		// R10G10B10A2
    extern CRGBBuffer g_LumaBuffer;
    extern CRGBBuffer g_TemporalColor[2];
    extern CRGBBuffer g_TemporalMinBound;
    extern CRGBBuffer g_TemporalMaxBound;

    //Arrays of RGB buffers that manage the bloom of the scene.
    extern CRGBBuffer g_aBloomUAV1[2];		// 640x384 (1/3)
    extern CRGBBuffer g_aBloomUAV2[2];		// 320x192 (1/6)  
    extern CRGBBuffer g_aBloomUAV3[2];		// 160x96  (1/12)
    extern CRGBBuffer g_aBloomUAV4[2];		// 80x48   (1/24)
    extern CRGBBuffer g_aBloomUAV5[2];		// 40x24   (1/48)

    extern CRGBBuffer g_LumaLR;
    extern CByteAddressBuffer g_Histogram;
    extern CByteAddressBuffer g_FXAAWorkQueue;
    extern CTypeBasedBuffer g_FXAAColorQueue;

    void InitializeRenderingBuffers(_In_ uint32_t NativeWidth, _In_ uint32_t NativeHeight );
    void ResizeDisplayDependentBuffers(_In_ uint32_t NativeWidth, _In_ uint32_t NativeHeight);
    void DestroyRenderingBuffers();

}
