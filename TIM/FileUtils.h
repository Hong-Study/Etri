#pragma once

class FileWriter
{
public:
	FileWriter();
	~FileWriter();
	void SetParentPath(std::string path);
	void FileStreamClose();

	bool FileStreamOpenWithCSV(std::string fileName, ePairState state, std::ios_base::openmode mode = std::ios_base::out | std::ios_base::app);

	template<typename T>
	bool WritePendingString(T* data);
	bool WritePairingString(StTifdData* tifd, StTirdData& tird);

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

template<typename T>
inline bool FileWriter::WritePendingString(T* data)
{
	if (!writer.is_open())
		return false;

	std::string time = std::format("{0}:{1}:{2}", data->stTime.hour, data->stTime.min, data->stTime.sec);
	std::string format = std::format("{0}, {1}, {2:0.5f}, {3:0.5f}, {4}, {5}\n", time.c_str(), data->deviceId, data->lat, data->lon, data->speed, data->sat);

	writer.write(format.c_str(), format.size());

	return true;
}