#include "PostEffectsRS.hlsli"
#include "ShaderUtility.hlsli"


cbuffer ExposureData : register(b1)
{
	float TargetLuminance;
	float AdaptationRate;
	float MinExposure;
	float MaxExposure;
	uint PixelCount;
};

ByteAddressBuffer Histogram : register(t0);
RWStructuredBuffer<float> Exposure : register(u0);

groupshared float gs_Accum[256];

[RootSignature(PostEffects_RootSig)]
[numthreads(128, 2, 1)]
void main(uint GlobalIllumination : SV_GroupIndex)
{
	float WeightedSum = (float)GlobalIllumination * (float)Histogram.Load(GlobalIllumination * 4);


	[unroll]
	for (uint i = 1; i < 256; i *= 2)
	{
		gs_Accum[GlobalIllumination] = WeightedSum;                 // Write
		GroupMemoryBarrierWithGroupSync();          // Sync
		WeightedSum += gs_Accum[(GlobalIllumination + i) % 256];    // Read
		GroupMemoryBarrierWithGroupSync();          // Sync
	}

	// If range of camera is black, no need to adjust exposure
	if (WeightedSum == 0.0)
		return;


	float MinLog = Exposure[4];
	float MaxLog = Exposure[5];
	float LogRange = Exposure[6];
	float RcpLogRange = Exposure[7];

	float weightedHistAvg = WeightedSum / (max(1, PixelCount - Histogram.Load(0))) - 1.0;
	float logAvgLuminance = exp2(weightedHistAvg / 254.0 * LogRange + MinLog);
	float targetExposure = TargetLuminance / logAvgLuminance;

	float exposure = Exposure[0];
	exposure = lerp(exposure, targetExposure, AdaptationRate);
	exposure = clamp(exposure, MinExposure, MaxExposure);

	if (GlobalIllumination == 0)
	{
		Exposure[0] = exposure;
		Exposure[1] = 2.0 / exposure;
		Exposure[2] = exposure;
		Exposure[3] = weightedHistAvg;

		// First attempt to recenter our histogram around the log-average.
		float biasToCenter = (floor(weightedHistAvg) - 128.0) / 255.0;
		if (abs(biasToCenter) > 0.1)
		{
			MinLog += biasToCenter * RcpLogRange;
			MaxLog += biasToCenter * RcpLogRange;
		}

		if (WaveIsFirstLane())
		{
			Exposure[4] = MinLog;
			Exposure[5] = MaxLog;
			Exposure[6] = LogRange;
			Exposure[7] = 1.0 / LogRange;
		}	
	}
}
