#pragma once

#include "Common.h"

#include "Entity.h"
#include "GeometryHelper.h"
#include "MapLoader.h"
#include "Settings.h"

class Item;
using ItemVec = vector<Item*>;
using ItemMap = map<uint, Item*>;
class Critter;
using CritterMap = map<uint, Critter*>;
using CritterVec = vector<Critter*>;
class Npc;
using NpcMap = map<uint, Npc*>;
using NpcVec = vector<Npc*>;
class Client;
using ClientMap = map<uint, Client*>;
using ClientVec = vector<Client*>;
class Map;
using MapVec = vector<Map*>;
using MapMap = map<uint, Map*>;
class Location;
using LocationVec = vector<Location*>;
using LocationMap = map<uint, Location*>;

struct StaticMap : public NonCopyable
{
    UCharVec SceneryData {};
    hash HashTiles {};
    hash HashScen {};
    CritterVec CrittersVec {};
    ItemVec AllItemsVec {};
    ItemVec HexItemsVec {};
    ItemVec ChildItemsVec {};
    ItemVec StaticItemsVec {};
    ItemVec TriggerItemsVec {};
    uchar* HexFlags {};
    vector<MapTile> Tiles {};
};

class Map : public Entity
{
    friend class MapManager;

public:
    Map(uint id, ProtoMap* proto, Location* location, StaticMap* static_map, MapSettings& sett);
    ~Map();
    StaticMap* GetStaticMap() { return staticMap; }

    void PlaceItemBlocks(ushort hx, ushort hy, Item* item);
    void RemoveItemBlocks(ushort hx, ushort hy, Item* item);

    void Process();
    void ProcessLoop(int index, uint time, uint tick);

    ProtoMap* GetProtoMap();
    Location* GetLocation();
    void SetLocation(Location* loc);

    void SetText(ushort hx, ushort hy, uint color, const string& text, bool unsafe_text);
    void SetTextMsg(ushort hx, ushort hy, uint color, ushort text_msg, uint num_str);
    void SetTextMsgLex(
        ushort hx, ushort hy, uint color, ushort text_msg, uint num_str, const char* lexems, ushort lexems_len);

    bool FindStartHex(ushort& hx, ushort& hy, uint multihex, uint seek_radius, bool skip_unsafe);

    void AddCritter(Critter* cr);
    void EraseCritter(Critter* cr);

    bool AddItem(Item* item, ushort hx, ushort hy);
    void SetItem(Item* item, ushort hx, ushort hy);
    void EraseItem(uint item_id);
    void SendProperty(NetProperty::Type type, Property* prop, Entity* entity);
    void ChangeViewItem(Item* item);
    void AnimateItem(Item* item, uchar from_frm, uchar to_frm);

    Item* GetItem(uint item_id);
    Item* GetItemHex(ushort hx, ushort hy, hash item_pid, Critter* picker);
    Item* GetItemGag(ushort hx, ushort hy);

    ItemVec GetItems();
    void GetItemsHex(ushort hx, ushort hy, ItemVec& items);
    void GetItemsHexEx(ushort hx, ushort hy, uint radius, hash pid, ItemVec& items);
    void GetItemsPid(hash pid, ItemVec& items);
    void GetItemsTrigger(ushort hx, ushort hy, ItemVec& items);

    bool IsPlaceForProtoItem(ushort hx, ushort hy, ProtoItem* proto_item);
    void RecacheHexFlags(ushort hx, ushort hy);
    ushort GetHexFlags(ushort hx, ushort hy);
    void SetHexFlag(ushort hx, ushort hy, uchar flag);
    void UnsetHexFlag(ushort hx, ushort hy, uchar flag);

    bool IsHexPassed(ushort hx, ushort hy);
    bool IsHexRaked(ushort hx, ushort hy);
    bool IsHexesPassed(ushort hx, ushort hy, uint radius);
    bool IsMovePassed(ushort hx, ushort hy, uchar dir, uint multihex);
    bool IsHexTrigger(ushort hx, ushort hy);
    bool IsHexCritter(ushort hx, ushort hy);
    bool IsHexGag(ushort hx, ushort hy);
    bool IsHexStaticTrigger(ushort hx, ushort hy);

    bool IsFlagCritter(ushort hx, ushort hy, bool dead);
    void SetFlagCritter(ushort hx, ushort hy, uint multihex, bool dead);
    void UnsetFlagCritter(ushort hx, ushort hy, uint multihex, bool dead);
    uint GetNpcCount(int npc_role, int find_type);
    Critter* GetCritter(uint crid);
    Critter* GetNpc(int npc_role, int find_type, uint skip_count);
    Critter* GetHexCritter(ushort hx, ushort hy, bool dead);
    void GetCrittersHex(ushort hx, ushort hy, uint radius, int find_type, CritterVec& critters);

    CritterVec GetCritters();
    ClientVec GetPlayers();
    NpcVec GetNpcs();
    CritterVec& GetCrittersRaw();
    ClientVec& GetPlayersRaw();
    NpcVec& GetNpcsRaw();
    uint GetCrittersCount();
    uint GetPlayersCount();
    uint GetNpcsCount();

    void SendEffect(hash eff_pid, ushort hx, ushort hy, ushort radius);
    void SendFlyEffect(
        hash eff_pid, uint from_crid, uint to_crid, ushort from_hx, ushort from_hy, ushort to_hx, ushort to_hy);

    bool SetScript(asIScriptFunction* func, bool first_time);

    void GetStaticItemTriggers(ushort hx, ushort hy, ItemVec& triggers);
    Item* GetStaticItem(ushort hx, ushort hy, hash pid);
    void GetStaticItemsHex(ushort hx, ushort hy, ItemVec& items);
    void GetStaticItemsHexEx(ushort hx, ushort hy, uint radius, hash pid, ItemVec& items);
    void GetStaticItemsByPid(hash pid, ItemVec& items);

    PROPERTIES_HEADER();
    CLASS_PROPERTY(uint, LoopTime1);
    CLASS_PROPERTY(uint, LoopTime2);
    CLASS_PROPERTY(uint, LoopTime3);
    CLASS_PROPERTY(uint, LoopTime4);
    CLASS_PROPERTY(uint, LoopTime5);
    CLASS_PROPERTY(string, FileDir);
    CLASS_PROPERTY(ushort, Width);
    CLASS_PROPERTY(ushort, Height);
    CLASS_PROPERTY(ushort, WorkHexX);
    CLASS_PROPERTY(ushort, WorkHexY);
    CLASS_PROPERTY(uint, LocId);
    CLASS_PROPERTY(uint, LocMapIndex);
    CLASS_PROPERTY(uchar, RainCapacity);
    CLASS_PROPERTY(int, CurDayTime);
    CLASS_PROPERTY(hash, ScriptId);
    CLASS_PROPERTY(CScriptArray*, DayTime); // 4 int
    CLASS_PROPERTY(CScriptArray*, DayColor); // 12 uchar
    CLASS_PROPERTY(bool, IsNoLogOut);

private:
    MapSettings& settings;
    GeometryHelper geomHelper;
    StaticMap* staticMap {};
    uchar* hexFlags {};
    int hexFlagsSize {};
    CritterVec mapCritters {};
    ClientVec mapPlayers {};
    NpcVec mapNpcs {};
    ItemVec mapItems {};
    ItemMap mapItemsById {};
    map<uint, ItemVec> mapItemsByHex {};
    map<uint, ItemVec> mapBlockLinesByHex {};
    Location* mapLocation {};
    uint loopLastTick[5] {};
};
