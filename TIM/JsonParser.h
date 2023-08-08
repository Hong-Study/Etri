#pragma once

enum class Type
{
	Int
	, Float
	, String
	, Nothing
};

class JsonParser
{
public:
	JsonParser(string fileName);
	void	Init();
	bool	StartParsing();

	bool	GetValue(const string& key, int32& input);
	bool	GetValue(const string& key, wstring& input);
	bool	GetValue(const string& key, string& input);
	bool	GetValue(const string& key, float& input);

private:
	void	CreateKeyValue(const string& str);
	void	CheckKeyValue(string key, string value, bool isString);
	void	InputNumberValue(string key, int32 value);
	void	InputStringValue(string key, string value);
	void	InputFloatValue(string key, float value);

private:
	string _dircectoryPath = "./";
	std::filesystem::path _totalPath;

	map<string, int32>	_intParser;
	map<string, float>	_floatParser;
	map<string, string> _stringParser;
	vector<char>		_checkList;
};