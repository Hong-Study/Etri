#include "pch.h"
#include "FileUtils.h"

FileWriter::FileWriter()
{
    parentPath = "";
}

FileWriter::~FileWriter()
{
    FileStreamClose();
}

void FileWriter::SetParentPath(std::string path)
{
    parentPath = path;
}

void FileWriter::FileStreamClose()
{
    if (writer.is_open())
        writer.close();
}

bool FileWriter::FileStreamOpenWithCSV(std::string fileName, ePairState state, std::ios_base::openmode mode)
{
    if (parentPath == "")
        return false;
    if (writer.is_open() == true)
        writer.close();

    std::filesystem::path totalPath = parentPath + fileName;
    if (std::filesystem::exists(totalPath.parent_path()) == false)
        std::filesystem::create_directories(totalPath.parent_path());

    writer.open(totalPath.string(), mode);
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

bool FileWriter::WritePendingString(const std::string time, const std::string deviceId, const float lat, const float lon, const float speed, const int32 sat)
{
    if (!writer.is_open())
        return false;

    std::string format = std::format("{0}, {1}, {2:0.5f}, {3:0.5f}, {4}, {5}\n", time.c_str(), deviceId.c_str(), lat, lon, speed, sat);
    writer.write(format.c_str(), format.size());

    return true;
}

bool FileWriter::WritePairingString(const std::string time, const std::string tifdDeviceId, const float tifdLat, const float tifdLon
    , const std::string tirdDeviceId, const float tirdLat, const float tirdLon, const int32 distance, const float speed, const int32 sat)
{
    if (!writer.is_open())
        return false;

    std::string format = std::format("{0}, {1}, {2:0.5f}, {3:0.5f}, {4}, {5:0.5f}, {6:0.5f}, {7}\n", time.c_str(), tifdDeviceId.c_str(), tifdLat, tifdLon
        , tirdDeviceId.c_str(), tirdLat, tirdLon, distance);
    writer.write(format.c_str(), format.size());

    return true;
}
