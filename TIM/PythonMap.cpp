#include "pch.h"
#include "PythonMap.h"
#include "WinApi.h"

// 스레드 Safe하게 만들어야하는데 어떻게 할 것인가?
// 구현 중점

void PythonMap::Init()
{
	Py_Initialize();

	_pFileName = PyUnicode_FromString("CreateMap");
	_pModule = PyImport_Import(_pFileName); // 생성한 PyObject pName을 import한다.
	if (_pModule == NULL) {
		CRASH("Can't open Module")
	}
	else
	{
		_pCreatePendingFunc = PyObject_GetAttrString(_pModule, "CreatePendingMap"); // 실행할 함수인 test_func을 PyObject에 전달한다. 
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

bool PythonMap::CreatePendingMapPng(float centerLat, float centerLong, float nowLat, float nowLong, string fileName)
{
	PyThreadState_Swap(NULL);
	PyThreadState_Swap(LMyThreadState);

	_pArgs = Py_BuildValue("(f, f, f, f, s)", centerLat, centerLong, nowLat, nowLong, fileName.c_str());
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

bool PythonMap::CreatePairingMapPng(float centerLat, float centerLong, float tifdLat, float tifdLong, float tirdLat, float tirdLong, string fileName)
{
	PyThreadState_Swap(NULL);
	PyThreadState_Swap(LMyThreadState);

	_pArgs = Py_BuildValue("(f, f, f, f, f, f, s)", centerLat, centerLong, tifdLat, tifdLong, tirdLat, tirdLong, fileName.c_str());
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
