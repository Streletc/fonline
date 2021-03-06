//      __________        ___               ______            _
//     / ____/ __ \____  / (_)___  ___     / ____/___  ____ _(_)___  ___
//    / /_  / / / / __ \/ / / __ \/ _ \   / __/ / __ \/ __ `/ / __ \/ _ \
//   / __/ / /_/ / / / / / / / / /  __/  / /___/ / / / /_/ / / / / /  __/
//  /_/    \____/_/ /_/_/_/_/ /_/\___/  /_____/_/ /_/\__, /_/_/ /_/\___/
//                                                  /____/
// FOnline Engine
// https://fonline.ru
// https://github.com/cvet/fonline
//
// MIT License
//
// Copyright (c) 2006 - present, Anton Tsvetinskiy aka cvet <cvet@tut.by>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include "Settings.h"
#include "FileSystem.h"
#include "StringUtils.h"
#include "Testing.h"
#include "WinApi_Include.h"

#include "imgui.h"

static void SetEntry(string& entry, const string& value)
{
    entry = value;
}
static void SetEntry(uchar& entry, const string& value)
{
    entry = _str(value).toInt();
}
static void SetEntry(short& entry, const string& value)
{
    entry = _str(value).toInt();
}
static void SetEntry(int& entry, const string& value)
{
    entry = _str(value).toInt();
}
static void SetEntry(uint& entry, const string& value)
{
    entry = _str(value).toInt();
}
static void SetEntry(bool& entry, const string& value)
{
    entry = _str(value).toBool();
}
static void SetEntry(float& entry, const string& value)
{
    entry = _str(value).toFloat();
}
static void SetEntry(vector<string>& entry, const string& value)
{
    entry.push_back(value);
}
static void SetEntry(vector<int>& entry, const string& value)
{
    entry.push_back(_str(value).toInt());
}
static void SetEntry(vector<uint>& entry, const string& value)
{
    entry.push_back(_str(value).toUInt());
}
static void SetEntry(vector<float>& entry, const string& value)
{
    entry.push_back(_str(value).toFloat());
}
static void SetEntry(vector<bool>& entry, const string& value)
{
    entry.push_back(_str(value).toBool());
}

static void DrawEntry(const char* name, const string& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, entry).c_str());
}
static void DrawEntry(const char* name, const uchar& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, entry).c_str());
}
static void DrawEntry(const char* name, const short& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, entry).c_str());
}
static void DrawEntry(const char* name, const int& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, entry).c_str());
}
static void DrawEntry(const char* name, const uint& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, entry).c_str());
}
static void DrawEntry(const char* name, const bool& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, entry).c_str());
}
static void DrawEntry(const char* name, const float& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, entry).c_str());
}
static void DrawEntry(const char* name, const vector<string>& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, "n/a").c_str());
}
static void DrawEntry(const char* name, const vector<int>& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, "n/a").c_str());
}
static void DrawEntry(const char* name, const vector<uint>& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, "n/a").c_str());
}
static void DrawEntry(const char* name, const vector<float>& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, "n/a").c_str());
}
static void DrawEntry(const char* name, const vector<bool>& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, "n/a").c_str());
}

static void DrawEditableEntry(const char* name, string& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, entry).c_str());
}
static void DrawEditableEntry(const char* name, uchar& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, entry).c_str());
}
static void DrawEditableEntry(const char* name, short& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, entry).c_str());
}
static void DrawEditableEntry(const char* name, int& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, entry).c_str());
}
static void DrawEditableEntry(const char* name, uint& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, entry).c_str());
}
static void DrawEditableEntry(const char* name, bool& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, entry).c_str());
}
static void DrawEditableEntry(const char* name, float& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, entry).c_str());
}
static void DrawEditableEntry(const char* name, vector<string>& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, "n/a").c_str());
}
static void DrawEditableEntry(const char* name, vector<int>& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, "n/a").c_str());
}
static void DrawEditableEntry(const char* name, vector<uint>& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, "n/a").c_str());
}
static void DrawEditableEntry(const char* name, vector<float>& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, "n/a").c_str());
}
static void DrawEditableEntry(const char* name, vector<bool>& entry)
{
    ImGui::TextUnformatted(_str("{}: {}", name, "n/a").c_str());
}

void GlobalSettings::ParseArgs(int argc, char** argv)
{
    DiskFileSystem::ResetCurDir();

    // Injected config
    static char internal_config[5022] = {
        "###InternalConfig###\0"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"};

    {
        ConfigFile config = ConfigFile(internal_config);
        for (auto& kv : config.GetApp(""))
            SetValue(kv.first, kv.second);
    }

    for (int i = 0; i < argc; i++)
    {
        // Skip path
        if (i == 0 && argv[0][0] != '-')
            continue;

        const_cast<vector<string>&>(CommandLineArgs).push_back(argv[i]);
        const_cast<string&>(CommandLine) += argv[i];
        if (i < argc - 1)
            const_cast<string&>(CommandLine) += " ";

        if (argv[i][0] == '-')
        {
            string key = _str("{}", argv[i]).trim().str().substr(1);
            if (key == "AddConfig")
            {
                if (i < argc - 1 && argv[i + 1][0] != '-')
                {
                    string path = _str(argv[i + 1]);
                    string dir = _str(path).extractDir();
                    string fname = _str(path).extractFileName();
                    DiskFileSystem::SetCurrentDir(dir);
                    DiskFile config_file = DiskFileSystem::OpenFile(fname, false);
                    if (config_file)
                    {
                        char* buf = new char[config_file.GetSize()];
                        if (config_file.Read(buf, config_file.GetSize()))
                        {
                            ConfigFile config = ConfigFile(buf);
                            for (auto& kv : config.GetApp(""))
                                SetValue(kv.first, kv.second);
                        }
                        delete[] buf;
                    }
                    DiskFileSystem::ResetCurDir();
                }
            }
            else
            {
                if (i < argc - 1 && argv[i + 1][0] != '-')
                    SetValue(key, _str("{}", argv[i + 1]).trim());
                else
                    SetValue(key, "1");
            }
        }
    }

#ifdef FO_WEB
    const_cast<bool&>(WebBuild) = true;
#else
    const_cast<bool&>(WebBuild) = false;
#endif
#ifdef FO_WINDOWS
    const_cast<bool&>(WindowsBuild) = true;
#else
    const_cast<bool&>(WindowsBuild) = false;
#endif
#ifdef FO_LINUX
    const_cast<bool&>(LinuxBuild) = true;
#else
    const_cast<bool&>(LinuxBuild) = false;
#endif
#ifdef FO_MAC
    const_cast<bool&>(MacOsBuild) = true;
#else
    const_cast<bool&>(MacOsBuild) = false;
#endif
#ifdef FO_ANDROID
    const_cast<bool&>(AndroidBuild) = true;
#else
    const_cast<bool&>(AndroidBuild) = false;
#endif
#ifdef FO_IOS
    const_cast<bool&>(IOsBuild) = true;
#else
    const_cast<bool&>(IOsBuild) = false;
#endif
    const_cast<bool&>(DesktopBuild) = (WindowsBuild || LinuxBuild || MacOsBuild);
    const_cast<bool&>(TabletBuild) = (AndroidBuild || IOsBuild);

#if defined(FO_WINDOWS) && !defined(WINRT)
    if (::GetSystemMetrics(SM_TABLETPC) != 0)
    {
        const_cast<bool&>(DesktopBuild) = false;
        const_cast<bool&>(TabletBuild) = true;
    }
#endif

    const_cast<int&>(MapDirCount) = (MapHexagonal ? 6 : 8);
}

void GlobalSettings::SetValue(const string& setting_name, string value)
{
    if (setting_name == "ServerDir")
        DiskFileSystem::ResolvePath(value);

#define SETTING(type, name, ...) \
    if (setting_name == #name) \
    { \
        SetEntry(const_cast<type&>(name), value); \
        return; \
    }
#define VAR_SETTING(type, name, ...) \
    if (setting_name == #name) \
    { \
        SetEntry(name, value); \
        return; \
    }
#define SETTING_GROUP(name, ...)
#define SETTING_GROUP_END()
#include "Settings_Include.h"
}

void GlobalSettings::Draw(bool editable)
{
#define SETTING(type, name, ...) \
    if (editable) \
        DrawEditableEntry(#name, const_cast<type&>(name)); \
    else \
        DrawEntry(#name, name)
#define VAR_SETTING(type, name, ...) \
    if (editable) \
        DrawEditableEntry(#name, name); \
    else \
        DrawEntry(#name, name)
#define SETTING_GROUP(name, ...)
#define SETTING_GROUP_END()
#include "Settings_Include.h"
}
