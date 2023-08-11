#pragma once

class FileWriter
{
public:
	FileWriter();
	~FileWriter();
	void SetParentPath(std::string path);
	void FileStreamClose();

	bool FileStreamOpenWithCSV(std::string fileName, ePairState state, std::ios_base::openmode mode = std::ios_base::out | std::ios_base::app);
	bool WritePendingString(const std::string time, const std::string deviceId, const float lat, const float lon, const float speed, const int32 sat);
	bool WritePairingString(const std::string time, const std::string tifdDeviceId, const float tifdLat, const float tifdLon
		, const std::string tirdDeviceId, const float tirdLat, const float tirdLon, const int32 distance, const float speed, const int32 sat);

	bool IsOpen() { return writer.is_open(); }
private:
	std::ofstream writer;
	std::string parentPath;

private:
	// 시간 Device Lat Long 스피드 위성
	std::string pendingSet = "Time,DeviceId,lat,long,Speed,Sat\n";

	// 시간 tifd tifd_lat tifd_long tird tird_lat tird_long distanec
	std::string pairingSet = "Time,TIFD,TIFD_Lat,TIFD_Long,TIRD,TIRD_Lat,TIRD_Long,Distance\n";
};