#include "pch.h"
#include "PythonMap.h"
#include "WinApi.h"

// ������ Safe�ϰ� �������ϴµ� ��� �� ���ΰ�?
// ���� ����

void PythonMap::Init()
{
	std::filesystem::path savePath = "./SaveMap";
	if (std::filesystem::exists(savePath) == false)
		std::filesystem::create_directory(savePath);

	Py_Initialize();
	_pFileName = PyUnicode_FromString("CreateMap");
	_pModule = PyImport_Import(_pFileName); // ������ PyObject pName�� import�Ѵ�.
	if (_pModule == NULL) {
		CRASH("Can't open Module")
	}
	else
	{
		_pCreatePendingFunc = PyObject_GetAttrString(_pModule, "CreatePendingMap"); // ������ �Լ��� test_func�� PyObject�� �����Ѵ�. 
		if (_pCreatePendingFunc == nullptr)
			CRASH("Can't Open Function");
		_pCreatePairingFunc = PyObject_GetAttrString(_pModule, "CreatePairingMap");
		if (_pCreatePairingFunc == nullptr)
			CRASH("Can't Open Function");
	}
	mainThreadState = PyThreadState_Get();
	LMyThreadState = PyThreadState_New(mainThreadState->interp);
}

void PythonMap::Clear()
{
	PyThreadState_Swap(mainThreadState);
	Py_Finalize();
}

bool PythonMap::CreatePendingMapPng(float centerLat, float centerLong, float nowLat, float nowLong, int32 zoomLevel)
{
	PyThreadState_Swap(NULL);
	PyThreadState_Swap(LMyThreadState);

	_pArgs = Py_BuildValue("(f, f, f, f, d)", centerLat, centerLong, nowLat, nowLong, zoomLevel);
	if (_pArgs == nullptr || _pCreatePendingFunc == nullptr)
		return false;
	_pValue = PyObject_CallObject(_pCreatePendingFunc, _pArgs);
	if (_pValue == nullptr)
		return false;
	return true;
}

bool PythonMap::CreatePendingMapPng(float centerLat, float centerLong, float nowLat, float nowLong)
{
	PyThreadState_Swap(NULL);
	PyThreadState_Swap(LMyThreadState);

	_pArgs = Py_BuildValue("(f, f, f, f)", centerLat, centerLong, nowLat, nowLong);
	if (_pArgs == nullptr || _pCreatePendingFunc == nullptr)
		return false;
	_pValue = PyObject_CallObject(_pCreatePendingFunc, _pArgs);
	if (_pValue == nullptr)
		return false;
	return true;
}

bool PythonMap::CreatePairingMapPng(float centerLat, float centerLong, float tifdLat, float tifdLong, float tirdLat, float tirdLong, int32 zoomLevel)
{
	PyThreadState_Swap(NULL);
	PyThreadState_Swap(LMyThreadState);

	_pArgs = Py_BuildValue("(f, f, f, f, f, f, d)", centerLat, centerLong, tifdLat, tifdLong, tirdLat, tirdLong, zoomLevel);
	if (_pArgs == nullptr || _pCreatePairingFunc == nullptr)
		return false;
	_pValue = PyObject_CallObject(_pCreatePairingFunc, _pArgs);
	if (_pValue == nullptr)
		return false;
	return true;
}

bool PythonMap::CreatePairingMapPng(float centerLat, float centerLong, float tifdLat, float tifdLong, float tirdLat, float tirdLong)
{
	PyThreadState_Swap(NULL);
	PyThreadState_Swap(LMyThreadState);

	_pArgs = Py_BuildValue("(f, f, f, f, f, f)", centerLat, centerLong, tifdLat, tifdLong, tirdLat, tirdLong);
	if (_pArgs == nullptr || _pCreatePairingFunc == nullptr)
		return false;
	_pValue = PyObject_CallObject(_pCreatePairingFunc, _pArgs);
	if (_pValue == nullptr)
		return false;
	return true;
}
