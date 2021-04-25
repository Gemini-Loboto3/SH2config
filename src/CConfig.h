#pragma once
#include "framework.h"
#include <vector>
#include <string>
#include <algorithm>
#include "tinyxml2.h"
#include "xxh3.h"

using namespace tinyxml2;
std::wstring MultiToWide_s(const char* multi);

class CConfigValue
{
public:
	void Parse(XMLElement& xml);

	std::string name;
	std::string id;
	bool is_default;
};

class CConfigOption
{
public:
	void Parse(XMLElement& xml);

	std::string name;		// option name
	std::string id, desc;	// string references
	UINT type;

	enum TYPE
	{
		TYPE_UNK,	// undefined behavior
		TYPE_LIST,	// list of options
		TYPE_CHECK,	// checkbox with true or false
		TYPE_PAD,	// controller enumerator
		TYPE_TEXT	// read only
	};

	std::vector<CConfigValue> value;
};

class CConfigSection
{
public:
	void Parse(XMLElement& xml);

	std::string name;	// section name
	std::vector<CConfigOption> option;	// options for this section
	std::string id;		// string references
};

// string pool
class CConfigStrings
{
public:
	class CConfigString
	{
	public:
		XXH64_hash_t hash;
		std::wstring str;
	};

	void PushString(const char* multis, const char* id)
	{
		CConfigString s;
		s.str = MultiToWide_s(multis);
		s.hash = XXH64(id, strlen(id), 0);

		str.push_back(s);
	}

	static bool sort(CConfigString& a, CConfigString& b)
	{
		return a.hash < b.hash;
	}

	int quickfind(XXH64_hash_t hash)
	{
		int first = 0,
			last = (int)str.size() - 1,
			middle = last / 2;

		while (first <= last)
		{
			if (str[middle].hash < hash)
				first = middle + 1;
			else if (str[middle].hash == hash)
				return middle;
			else last = middle - 1;

			middle = (first + last) / 2;
		}

		return -1;
	}

	void Sort()
	{
		std::sort(str.begin(), str.end(), sort);
	}

	std::wstring Find(XXH64_hash_t id)
	{
		auto f = quickfind(id);
		if (f >= 0) return str[f].str;

		return std::wstring(L"");
	}

	std::wstring Find(std::string id)
	{
		auto hash = XXH64(id.c_str(), id.size(), 0);

		auto f = quickfind(hash);
		if (f >= 0) return str[f].str;

		return std::wstring(L"");
	}

	std::vector<CConfigString> str;
};

// group manager
class CConfigGroup
{
public:
	class CConfigSubOpt
	{
	public:
		void Set(std::string section, std::string option)
		{
			sec = XXH64(section.c_str(), section.size(), 0);
			op = XXH64(option.c_str(), option.size(), 0);
		}

		XXH64_hash_t sec, op;		// less memory with hashes in this case
	};
	class CConfigSub
	{
	public:
		void Parse(XMLElement& xml);
		std::vector<CConfigSubOpt> opt;
		std::string id;
	};
	void Parse(XMLElement& xml);

	std::string id;
	std::vector<CConfigSub> sub;
};

class CConfig
{
public:
	void ParseXml();

	std::vector<CConfigSection> section;	// ini hierarchy
	std::vector<CConfigGroup> group;		// ini groups, represented as tabs on interface
	CConfigStrings string;					// string pool

	void FindSectionAndOption(XXH64_hash_t ss, XXH64_hash_t sh, int &found_sec, int &found_opt)
	{
		for (size_t i = 0, si = section.size(); i < si; i++)
		{
			auto xs = XXH64(section[i].name.c_str(), section[i].name.size(), 0);
			for (size_t j = 0, sj = section[i].option.size(); j < sj; j++)
			{
				auto xh = XXH64(section[i].option[j].name.c_str(), section[i].option[j].name.size(), 0);
				if (xs == ss && xh == sh)
				{
					found_sec = i;
					found_opt = j;
					return;
				}
			}
		}
		found_sec = 0;
		found_opt = 0;
	}

	std::wstring GetSectionString(int sec);
	std::wstring GetOptionString(int sec, int opt);
	std::wstring GetOptionDesc(int sec, int opt);
	std::wstring GetValueString(int sec, int opt, int val);

	std::wstring GetGroupString(int sec);
	std::wstring GetGroupLabel(int sec, int sub);
};

