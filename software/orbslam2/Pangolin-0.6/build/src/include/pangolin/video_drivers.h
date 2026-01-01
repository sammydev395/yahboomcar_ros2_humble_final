// CMake generated file. Do Not Edit.

#pragma once

namespace pangolin {

void RegisterTestVideoFactory();
void RegisterImagesVideoFactory();
void RegisterImagesVideoOutputFactory();
void RegisterSplitVideoFactory();
void RegisterTruncateVideoFactory();
void RegisterPvnVideoFactory();
void RegisterPangoVideoFactory();
void RegisterPangoVideoOutputFactory();
void RegisterDebayerVideoFactory();
void RegisterShiftVideoFactory();
void RegisterMirrorVideoFactory();
void RegisterUnpackVideoFactory();
void RegisterPackVideoFactory();
void RegisterJoinVideoFactory();
void RegisterMergeVideoFactory();
void RegisterJsonVideoFactory();
void RegisterThreadVideoFactory();
void RegisterFirewireVideoFactory();
void RegisterV4lVideoFactory();
void RegisterOpenNiVideoFactory();
void RegisterOpenNi2VideoFactory();
void RegisterUvcVideoFactory();

inline bool LoadBuiltInVideoDrivers()
{
    RegisterTestVideoFactory();
    RegisterImagesVideoFactory();
    RegisterImagesVideoOutputFactory();
    RegisterSplitVideoFactory();
    RegisterTruncateVideoFactory();
    RegisterPvnVideoFactory();
    RegisterPangoVideoFactory();
    RegisterPangoVideoOutputFactory();
    RegisterDebayerVideoFactory();
    RegisterShiftVideoFactory();
    RegisterMirrorVideoFactory();
    RegisterUnpackVideoFactory();
    RegisterPackVideoFactory();
    RegisterJoinVideoFactory();
    RegisterMergeVideoFactory();
    RegisterJsonVideoFactory();
    RegisterThreadVideoFactory();
    RegisterFirewireVideoFactory();
    RegisterV4lVideoFactory();
    RegisterOpenNiVideoFactory();
    RegisterOpenNi2VideoFactory();
    RegisterUvcVideoFactory();
    return true;
}

} // pangolin
