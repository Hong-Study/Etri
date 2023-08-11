#pragma once

class PythonMap
{
	SINGLETON(PythonMap)
public:
	void Init();
	void Clear();
	bool CreatePendingMapPng(float centerLat, float centerLong, float nowLat, float nowLong, string fileName);
	bool CreatePendingMapPng(float centerLat, float centerLong, float nowLat, float nowLong);
	bool CreatePairingMapPng(float centerLat, float centerLong, float tifdLat, float tifdLong, float tirdLat, float tirdLong, string fileName);
	bool CreatePairingMapPng(float centerLat, float centerLong, float tifdLat, float tifdLong, float tirdLat, float tirdLong);

private:
	PyObject* _pFileName = nullptr;
	PyObject* _pModule = nullptr;
	PyObject* _pCreatePendingFunc = nullptr;
	PyObject* _pCreatePairingFunc = nullptr;
	PyObject* _pArgs = nullptr;
	PyObject* _pValue = nullptr;

public:
	PyThreadState* mainThreadState = nullptr;
};