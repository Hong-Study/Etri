#include "pch.h"
#include "FileUtils.h"

FileWriter::FileWriter()
{
   
}

FileWriter::~FileWriter()
{
    FileStreamClose();
}

void FileWriter::FileStreamClose()
{
    if (writer.is_open())
        writer.close();
}

bool FileWriter::FileStreamOpenWithCSV(int32 month, int32 day, int32 time,std::string deviceId, Device device, ePairState state, std::ios_base::openmode mode)
{
    if (writer.is_open() == true)
        writer.close();

    if (device == Device::DeviceTIFD)
    {
        if (state == ePairState::PairState_Pair)
            filePath = std::format("PAIRING/{0}/{1}/{2}/{3}.csv", month, day, time, deviceId.c_str());
        else if (state == ePairState::PairState_Unpair)
            filePath = std::format("TIFD/{0}/{1}/{2}/{3}.csv", month, day, time, deviceId.c_str());
    }
    else if (device == Device::DeviceTIRD)
        filePath = std::format("TIRD/{0}/{1}/{2}/{3}.csv", month, day, time, deviceId.c_str());
    else
        return false;

    std::filesystem::path totalPath = GLogPos + filePath;

    if (std::filesystem::exists(totalPath.parent_path()) == false)
        std::filesystem::create_directories(totalPath.parent_path());

    writer.open(totalPath.c_str(), mode);
    if (writer.is_open() == false)
        return false;

    if (state == ePairState::PairState_Pair)
        writer << pairingSet;
    else if (state == ePairState::PairState_Unpair)
        writer << pendingSet;
    else
    {
        writer.close();
        return false;
    }

    return true;
}

bool FileWriter::WritePairingString(StTifdData* tifd, StTirdData& tird)
{
    if (!writer.is_open())
        return false;

    std::string time = std::format("{0}:{1}:{2}", tifd->stTime.hour, tifd->stTime.min, tifd->stTime.sec);

    std::string format = std::format("{0}, {1}, {2:0.8f}, {3:0.8f}, {4:0.5f}, {5}, {6}, {7:0.8f}, {8:0.8f}, {9:0.5}, {10}, {11}\n"
        , time.c_str(), tifd->deviceId, tifd->lat, tifd->lon, tifd->speed, tifd->sat, tird.deviceId, tird.lat, tird.lon, tird.speed, tird.sat, tifd->distance);

    writer.write(format.c_str(), format.size());

    return true;
}
