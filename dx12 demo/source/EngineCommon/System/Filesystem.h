#pragma once
#include <filesystem>
#include "EngineCommon/util/Util.h"

namespace fs = std::filesystem;
using path = fs::path;
extern path g_homeDir;
extern path g_demoDir;
extern path g_ShaderDir;

namespace engine::system
{
class Filepath
{
public:
	Filepath(std::string name);
	Filepath(fs::path path);
	path getPath() { return m_path; }
	std::string readFile();
	std::wstring wstr() { return m_path.c_str(); }
	std::string str() { return util::to_string(m_path); }
	std::wstring wfilename() { return m_path.filename(); }
	std::string filename() { return util::to_string(m_path.filename()); }
private:
	path m_path;
};


};
