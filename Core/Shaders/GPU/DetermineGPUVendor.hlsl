/*
    Determine GPU Vendor HLSL Library

    Morgan Ruffell - Ruffell Interactive - 2022
    HLSL - SM6.2
*/

struct WaveProgrammingData
{
    bool UsingNvidiaGraphicsCard = false;
    bool UsingAMDGraphicsCard = false;
    bool UsingIntelGraphicsCard = false;
    bool CannotDetermineGraphicsCardVendor = false;

    uint LaneCount;

};

ConstantBuffer<WaveProgrammingData> WaveData : register(b0);

//This determines how many waves each GPU will have for each frame.
//These will contain lane.
void DetermineVendor()
{
    //I want this to take place on the first thread/lane
    if(WaveIsFirstLane())
    {
        //Each graphics vendor has a different wave count for
        //thier GPUs -- NVidia is 32
        if(WaveGetLaneCount() == 32)
        {
            WaveData.UsingNvidiaGraphicsCard = true;
            WaveData.LaneCount = WaveGetLaneCount(); 
        }

        //AMD is 64
        else if (WaveGetLaneCount() == 64)
        {
            WaveData.UsingAMDGraphicsCard = true;

        }

        //To do: Add in intel integrated solution -- Why you'd be running creative on a laptop is anyones guess
        else
        {
            WaveData.CannotDetermineGraphicsCardVendor = true;
        }
    }
    else
    {
        WaveData.CannotDetermineGraphicsCardVendor = true;
    }

}