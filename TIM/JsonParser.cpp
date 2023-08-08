#include "pch.h"
#include "JsonParser.h"
#include <type_traits>

JsonParser::JsonParser(string fileName)
	: _totalPath(_dircectoryPath + fileName)
{
	Init();
}

void JsonParser::Init()
{
	_checkList.push_back(' ');
	_checkList.push_back('\n');
	_checkList.push_back('\t');
	_checkList.push_back(',');
	_checkList.push_back('\"');
}

bool JsonParser::StartParsing()
{
	if (std::filesystem::exists(_totalPath) == false)
		return false;

	std::ifstream ifs(_totalPath.c_str());

	string str;
	while (std::getline(ifs, str))
	{
		if (str == "{")
			continue;

		if (str == "}")
			break;

		CreateKeyValue(str);
	}

	ifs.close();
	return true;
}

bool JsonParser::GetValue(const string& key, int32& input)
{
	if (_intParser.find(key) == _intParser.end())
		return false;

	input = _intParser[key];
	return true;
}

bool JsonParser::GetValue(const string& key, wstring& input)
{
	if (_stringParser.find(key) == _stringParser.end())
		return false;

	input = StringToWstring(_stringParser[key]);
	return true;
}

bool JsonParser::GetValue(const string& key, string& input)
{
	if (_stringParser.find(key) == _stringParser.end())
		return false;

	input = _stringParser[key];
	return true;
}

bool JsonParser::GetValue(const string& key, float& input)
{
	if (_floatParser.find(key) == _floatParser.end())
		return false;

	input = _floatParser[key];
	return true;
}

void JsonParser::CreateKeyValue(const string& str)
{
	string key = "";
	string value = "";
	bool check = true;
	bool isString = false;
	for (int i = 0;i < str.size();i++)
	{
		char c = str[i];
		bool isRight = false;

		if (check == false && c == '\"')
		{
			isString = true;
			continue;
		}
			
		for (char ch : _checkList)
		{
			if (c == ch)
			{
				isRight = true;
				break;
			}
		}

		if (isRight)
			continue;

		if (c == ':')
		{
			check = false;
			continue;
		}

		if (check)
			key += c;
		else
			value += c;
	}
	CheckKeyValue(key, value, isString);
}

void JsonParser::CheckKeyValue(string key, string value, bool isString)
{
	if (isString)
		InputStringValue(key, value);
	else
	{
		bool isStringNumber = true;
		bool isStringFloat = false;
		for (int i = 0;i < value.size();i++)
		{
			if (isdigit(value[i]) == 0)
			{
				if (value[i] == '.')
					isStringFloat = true;
				else
					isStringNumber = false;

				break;
			}
		}

		if (isStringFloat)
			InputFloatValue(key, std::stof(value));
		else if (isStringNumber)
			InputNumberValue(key, std::stoi(value));

	}
}

void JsonParser::InputNumberValue(string key, int32 value)
{
	_intParser[key] = value;
}

void JsonParser::InputStringValue(string key, string value)
{
	_stringParser[key] = value;
}

void JsonParser::InputFloatValue(string key, float value)
{
	_floatParser[key] = value;
}
