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

#include "ProtoManager.h"
#include "FileSystem.h"
#include "GenericUtils.h"
#include "Log.h"
#include "StringUtils.h"
#include "Testing.h"

template<class T>
static void WriteProtosToBinary(UCharVec& data, const map<hash, T*>& protos)
{
    WriteData(data, (uint)protos.size());
    for (auto& kv : protos)
    {
        hash proto_id = kv.first;
        ProtoEntity* proto_item = kv.second;
        WriteData(data, proto_id);

        WriteData(data, (ushort)proto_item->Components.size());
        for (hash component : proto_item->Components)
            WriteData(data, component);

        PUCharVec* props_data;
        UIntVec* props_data_sizes;
        proto_item->Props.StoreData(true, &props_data, &props_data_sizes);
        WriteData(data, (ushort)props_data->size());
        for (size_t i = 0; i < props_data->size(); i++)
        {
            uint cur_size = props_data_sizes->at(i);
            WriteData(data, cur_size);
            WriteDataArr(data, props_data->at(i), cur_size);
        }
    }
}

template<class T>
static void ReadProtosFromBinary(UCharVec& data, uint& pos, map<hash, T*>& protos)
{
    PUCharVec props_data;
    UIntVec props_data_sizes;
    uint protos_count = ReadData<uint>(data, pos);
    for (uint i = 0; i < protos_count; i++)
    {
        hash proto_id = ReadData<hash>(data, pos);
        T* proto = new T(proto_id);

        ushort components_count = ReadData<ushort>(data, pos);
        for (ushort j = 0; j < components_count; j++)
            proto->Components.insert(ReadData<hash>(data, pos));

        uint data_count = ReadData<ushort>(data, pos);
        props_data.resize(data_count);
        props_data_sizes.resize(data_count);
        for (uint j = 0; j < data_count; j++)
        {
            props_data_sizes[j] = ReadData<uint>(data, pos);
            props_data[j] = ReadDataArr<uchar>(data, props_data_sizes[j], pos);
        }
        proto->Props.RestoreData(props_data, props_data_sizes);
        RUNTIME_ASSERT(!protos.count(proto_id));
        protos.insert(std::make_pair(proto_id, proto));
    }
}

static void InsertMapValues(const StrMap& from_kv, StrMap& to_kv, bool overwrite)
{
    for (auto& kv : from_kv)
    {
        RUNTIME_ASSERT(!kv.first.empty());
        if (kv.first[0] != '$')
        {
            if (overwrite)
                to_kv[kv.first] = kv.second;
            else
                to_kv.insert(std::make_pair(kv.first, kv.second));
        }
        else if (kv.first == "$Components" && !kv.second.empty())
        {
            if (!to_kv.count("$Components"))
                to_kv["$Components"] = kv.second;
            else
                to_kv["$Components"] += " " + kv.second;
        }
    }
}

template<class T>
static void ParseProtos(FileManager& file_mngr, const string& ext, const string& app_name, map<hash, T*>& protos)
{
    WriteLog("Load protos '{}'.\n", ext);

    // Collect data
    FileCollection files = file_mngr.FilterFiles(ext);
    map<hash, StrMap> files_protos;
    map<hash, map<string, StrMap>> files_texts;
    while (files.MoveNext())
    {
        File file = files.GetCurFile();
        ConfigFile fopro(file.GetCStr());
        WriteLog("Load proto '{}'.\n", file.GetName());

        PStrMapVec protos_data;
        fopro.GetApps(app_name, protos_data);
        if (std::is_same<T, ProtoMap>::value && protos_data.empty())
            fopro.GetApps("Header", protos_data);

        for (auto& pkv : protos_data)
        {
            auto& kv = *pkv;
            string name = (kv.count("$Name") ? kv["$Name"] : file.GetName());
            hash pid = _str(name).toHash();
            if (files_protos.count(pid))
                throw ProtoManagerException("Proto already loaded", name);

            files_protos.insert(std::make_pair(pid, kv));

            StrSet apps;
            fopro.GetAppNames(apps);
            for (auto& app_name : apps)
            {
                if (app_name.size() == 9 && app_name.find("Text_") == 0)
                {
                    if (!files_texts.count(pid))
                    {
                        map<string, StrMap> texts;
                        files_texts.insert(std::make_pair(pid, texts));
                    }
                    files_texts[pid].insert(std::make_pair(app_name, fopro.GetApp(app_name)));
                }
            }
        }

        if (protos_data.empty())
            throw ProtoManagerException("File does not contain any proto", file.GetName());
    }

    // Injection
    auto injection = [&files_protos](const char* key_name, bool overwrite) {
        for (auto& inject_kv : files_protos)
        {
            if (inject_kv.second.count(key_name))
            {
                for (auto& inject_name : _str(inject_kv.second[key_name]).split(' '))
                {
                    if (inject_name == "All")
                    {
                        for (auto& kv : files_protos)
                            if (kv.first != inject_kv.first)
                                InsertMapValues(inject_kv.second, kv.second, overwrite);
                    }
                    else
                    {
                        hash inject_name_hash = _str(inject_name).toHash();
                        if (!files_protos.count(inject_name_hash))
                            throw ProtoManagerException("Proto not found for injection from another proto",
                                inject_name.c_str(), _str().parseHash(inject_kv.first));
                        InsertMapValues(inject_kv.second, files_protos[inject_name_hash], overwrite);
                    }
                }
            }
        }
    };
    injection("$Inject", false);

    // Protos
    for (auto& kv : files_protos)
    {
        hash pid = kv.first;
        string base_name = _str().parseHash(pid);
        RUNTIME_ASSERT(protos.count(pid) == 0);

        // Fill content from parents
        StrMap final_kv;
        std::function<void(const string&, StrMap&)> fill_parent = [&fill_parent, &base_name, &files_protos, &final_kv](
                                                                      const string& name, StrMap& cur_kv) {
            const char* parent_name_line = (cur_kv.count("$Parent") ? cur_kv["$Parent"].c_str() : "");
            for (auto& parent_name : _str(parent_name_line).split(' '))
            {
                hash parent_pid = _str(parent_name).toHash();
                auto parent = files_protos.find(parent_pid);
                if (parent == files_protos.end())
                {
                    if (base_name == name)
                        throw ProtoManagerException("Proto fail to load parent", base_name, parent_name);
                    else
                        throw ProtoManagerException(
                            "Proto fail to load parent for another proto", base_name, parent_name, name);
                }

                fill_parent(parent_name, parent->second);
                InsertMapValues(parent->second, final_kv, true);
            }
        };
        fill_parent(base_name, kv.second);

        // Actual content
        InsertMapValues(kv.second, final_kv, true);

        // Final injection
        injection("$InjectOverride", true);

        // Create proto
        T* proto = new T(pid);
        if (!proto->Props.LoadFromText(final_kv))
            throw ProtoManagerException("Proto item fail to load properties", base_name);

        // Components
        if (final_kv.count("$Components"))
        {
            for (const string& component_name : _str(final_kv["$Components"]).split(' '))
            {
                hash component_name_hash = _str(component_name).toHash();
                if (!proto->Props.GetRegistrator()->IsComponentRegistered(component_name_hash))
                    throw ProtoManagerException("Proto item has invalid component", base_name, component_name);
                proto->Components.insert(component_name_hash);
            }
        }

        // Add to collection
        protos.insert(std::make_pair(pid, proto));
    }

    // Texts
    for (auto& kv : files_texts)
    {
        T* proto = protos[kv.first];
        RUNTIME_ASSERT(proto);
        for (auto& text : kv.second)
        {
            FOMsg temp_msg;
            temp_msg.LoadFromMap(text.second);

            FOMsg* msg = new FOMsg();
            uint str_num = 0;
            while ((str_num = temp_msg.GetStrNumUpper(str_num)))
            {
                uint count = temp_msg.Count(str_num);
                uint new_str_num = str_num;
                if (std::is_same<T, ProtoItem>::value)
                    new_str_num = ITEM_STR_ID(proto->ProtoId, str_num);
                else if (std::is_same<T, ProtoCritter>::value)
                    new_str_num = CR_STR_ID(proto->ProtoId, str_num);
                else if (std::is_same<T, ProtoLocation>::value)
                    new_str_num = LOC_STR_ID(proto->ProtoId, str_num);
                for (uint n = 0; n < count; n++)
                    msg->AddStr(new_str_num, temp_msg.GetStr(str_num, n));
            }

            proto->TextsLang.push_back(*(uint*)text.first.substr(5).c_str());
            proto->Texts.push_back(msg);
        }
    }
}

void ProtoManager::LoadProtosFromFiles(FileManager& file_mngr)
{
    WriteLog("Load protos from files.\n");

    ParseProtos(file_mngr, "foitem", "ProtoItem", itemProtos);
    ParseProtos(file_mngr, "focr", "ProtoCritter", crProtos);
    ParseProtos(file_mngr, "fomap", "ProtoMap", mapProtos);
    ParseProtos(file_mngr, "foloc", "ProtoLocation", locProtos);

    // Mapper collections
    for (auto& kv : itemProtos)
    {
        if (!kv.second->Components.empty())
            kv.second->CollectionName = _str().parseHash(*kv.second->Components.begin()).lower();
        else
            kv.second->CollectionName = "other";
    }
    for (auto& kv : crProtos)
    {
        kv.second->CollectionName = "all";
    }

    // Check player proto
    if (!crProtos.count(_str("Player").toHash()))
        throw ProtoManagerException("Player proto 'Player.focr' not loaded");

    // Check maps for locations
    for (auto& kv : locProtos)
        for (hash map_pid : kv.second->GetMapProtos())
            if (!mapProtos.count(map_pid))
                throw ProtoManagerException(
                    "Proto map not found for proto location", _str().parseHash(map_pid), kv.second->GetName());
}

UCharVec ProtoManager::GetProtosBinaryData()
{
    UCharVec data;
    WriteProtosToBinary(data, itemProtos);
    WriteProtosToBinary(data, crProtos);
    WriteProtosToBinary(data, mapProtos);
    WriteProtosToBinary(data, locProtos);
    Compressor::Compress(data);
    return data;
}

void ProtoManager::LoadProtosFromBinaryData(UCharVec& data)
{
    if (data.empty())
        return;
    if (!Compressor::Uncompress(data, 15))
        return;

    uint pos = 0;
    ReadProtosFromBinary(data, pos, itemProtos);
    ReadProtosFromBinary(data, pos, crProtos);
    ReadProtosFromBinary(data, pos, mapProtos);
    ReadProtosFromBinary(data, pos, locProtos);
}

template<typename T>
static int ValidateProtoResourcesExt(map<hash, T*>& protos, HashSet& hashes)
{
    int errors = 0;
    for (auto& kv : protos)
    {
        T* proto = kv.second;
        PropertyRegistrator* registrator = proto->Props.GetRegistrator();
        for (uint i = 0; i < registrator->GetCount(); i++)
        {
            Property* prop = registrator->Get(i);
            if (prop->IsResource())
            {
                hash h = proto->Props.template GetValue<hash>(prop);
                if (h && !hashes.count(h))
                {
                    WriteLog("Resource '{}' not found for property '{}' in prototype '{}'.\n", _str().parseHash(h),
                        prop->GetName(), proto->GetName());
                    errors++;
                }
            }
        }
    }
    return errors;
}

bool ProtoManager::ValidateProtoResources(StrVec& resource_names)
{
    HashSet hashes;
    for (auto& name : resource_names)
        hashes.insert(_str(name).toHash());

    int errors = 0;
    errors += ValidateProtoResourcesExt(itemProtos, hashes);
    errors += ValidateProtoResourcesExt(crProtos, hashes);
    errors += ValidateProtoResourcesExt(mapProtos, hashes);
    errors += ValidateProtoResourcesExt(locProtos, hashes);
    return errors == 0;
}

ProtoItem* ProtoManager::GetProtoItem(hash pid)
{
    auto it = itemProtos.find(pid);
    return it != itemProtos.end() ? it->second : nullptr;
}

ProtoCritter* ProtoManager::GetProtoCritter(hash pid)
{
    auto it = crProtos.find(pid);
    return it != crProtos.end() ? it->second : nullptr;
}

ProtoMap* ProtoManager::GetProtoMap(hash pid)
{
    auto it = mapProtos.find(pid);
    return it != mapProtos.end() ? it->second : nullptr;
}

ProtoLocation* ProtoManager::GetProtoLocation(hash pid)
{
    auto it = locProtos.find(pid);
    return it != locProtos.end() ? it->second : nullptr;
}

const ProtoItemMap& ProtoManager::GetProtoItems()
{
    return itemProtos;
}

const ProtoCritterMap& ProtoManager::GetProtoCritters()
{
    return crProtos;
}

const ProtoMapMap& ProtoManager::GetProtoMaps()
{
    return mapProtos;
}

const ProtoLocationMap& ProtoManager::GetProtoLocations()
{
    return locProtos;
}
