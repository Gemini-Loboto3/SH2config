#include "CConfig.h"

/////////////////////////////////////////////////
// Various helpers
std::string SAFESTR(const char *X)
{
	if (!X)
		return std::string("");

	return std::string(X);
}

// create a wide string from utf8
wchar_t* MultiToWide(const char* multi)
{
	// gather size of the new string and create a buffer
	size_t size = MultiByteToWideChar(CP_UTF8, 0, multi, -1, NULL, 0);
	wchar_t* wide = new wchar_t[size];
	// fill allocated string with converted data
	MultiByteToWideChar(CP_UTF8, 0, multi, -1, wide, size);

	return wide;
}

// create an std::wstring from utf8
std::wstring MultiToWide_s(const char* multi)
{
	if (!multi)
		return std::wstring(L"");

	wchar_t* wide = MultiToWide(multi);
	std::wstring wstr = wide;
	delete[] wide;

	return wstr;
}

// crate an std::wstring from utf8 str::string
std::wstring MultiToWide_s(std::string& multi)
{
	return MultiToWide_s(multi.c_str());
}

/////////////////////////////////////////////////
// Actual configuration
void CConfig::ParseXml()
{
	XMLDocument xml;
	xml.LoadFile("config.xml");

	auto root = xml.RootElement();
	auto s = root->FirstChildElement("Sections");
	auto sec = s->FirstChildElement("Section");
	while (sec)
	{
		auto name = SAFESTR(sec->Attribute("name"));
		CConfigSection ssec;
		ssec.name = name;
		ssec.Parse(*sec);
		section.push_back(ssec);

		sec = sec->NextSiblingElement();
	}

	auto g = root->FirstChildElement("Groups");
	auto gp = g->FirstChildElement("Group");
	while (gp)
	{
		CConfigGroup gg;
		gg.Parse(*gp);
		group.push_back(gg);

		gp = gp->NextSiblingElement("Group");
	}

	s = root->FirstChildElement("Strings");
	sec = s->FirstChildElement("S");
	while (sec)
	{
		auto str = SAFESTR(sec->GetText());
		auto id = SAFESTR(sec->Attribute("id"));

		string.PushString(str.c_str(), id.c_str());
		//string.push_back(MultiToWide_s(str));

		sec = sec->NextSiblingElement();
	}
	string.Sort();
}

std::wstring CConfig::GetSectionString(int sec)
{
	auto id = string.Find(section[sec].id);
	if (id.size() == 0)
		return MultiToWide_s(section[sec].name);

	return id;
}

std::wstring CConfig::GetGroupString(int sec)
{
	auto id = string.Find(group[sec].id);
	if (id.size() == 0)
		return MultiToWide_s(group[sec].id);

	return id;
}

std::wstring CConfig::GetGroupLabel(int sec, int sub)
{
	auto id = string.Find(group[sec].sub[sub].id);
	if (id.size() == 0)
		return MultiToWide_s(group[sec].sub[sub].id);

	return id;
}

std::wstring CConfig::GetOptionString(int sec, int opt)
{
	auto id = string.Find(section[sec].option[opt].id);
	if (id.size() == 0)
		return MultiToWide_s(section[sec].option[opt].name);

	return id;
}
std::wstring CConfig::GetOptionDesc(int sec, int opt) { return L""; }
std::wstring CConfig::GetValueString(int sec, int opt, int val)
{
	auto id = string.Find(section[sec].option[opt].value[val].id);
	if (id.size() == 0)
		return MultiToWide_s(section[sec].option[opt].value[val].name);

	return id;
}

/////////////////////////////////////////////////
void CConfigSection::Parse(XMLElement& xml)
{
	name = SAFESTR(xml.Attribute("name"));
	id = SAFESTR(xml.Attribute("id"));

	auto s = xml.FirstChildElement("Option");
	while (s)
	{
		CConfigOption opt;
		opt.Parse(*s);
		option.push_back(opt);

		s = s->NextSiblingElement("Option");
	}
}

/////////////////////////////////////////////////
void CConfigOption::Parse(XMLElement& xml)
{
	name = SAFESTR(xml.Attribute("name"));
	auto t = xml.Attribute("type");
	type = TYPE_UNK;
	if (t)
	{
		if (strcmp(t, "list") == 0)
			type = TYPE_LIST;
		else if (strcmp(t, "check") == 0)
			type = TYPE_CHECK;
		else if (strcmp(t, "invisible") == 0)
			type = TYPE_TEXT;
		else if (strcmp(t, "pad") == 0)
			type = TYPE_PAD;
	}

	id = SAFESTR(xml.Attribute("id"));
	desc = SAFESTR(xml.Attribute("desc"));

	auto s = xml.FirstChildElement("Value");
	while (s)
	{
		CConfigValue val;
		val.Parse(*s);
		value.push_back(val);

		s = s->NextSiblingElement("Value");
	}
}

/////////////////////////////////////////////////
void CConfigValue::Parse(XMLElement& xml)
{
	name = SAFESTR(xml.Attribute("tip"));
	id = SAFESTR(xml.Attribute("id"));
	is_default = xml.IntAttribute("default", 0);
}

/////////////////////////////////////////////////
void CConfigGroup::Parse(XMLElement& xml)
{
	//auto gp = xml.FirstChildElement("Groups");
	id = SAFESTR(xml.Attribute("id"));

	auto o = xml.FirstChildElement("Sub");
	while (o)
	{
		CConfigSub s;
		s.Parse(*o);
		s.id = SAFESTR(o->Attribute("id"));
		sub.push_back(s);
		o = o->NextSiblingElement("Sub");
	}
}

/////////////////////////////////////////////////
void CConfigGroup::CConfigSub::Parse(XMLElement& xml)
{
	auto o = xml.FirstChildElement("Opt");
	while (o)
	{
		CConfigSubOpt op;
		std::string xs = SAFESTR(o->Attribute("sec"));
		std::string xo = SAFESTR(o->Attribute("op"));
		op.Set(xs, xo);
		opt.push_back(op);
		o = o->NextSiblingElement("Opt");
	}
}