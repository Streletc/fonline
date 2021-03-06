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

#include "CritterView.h"
#include "GenericUtils.h"
#include "ItemView.h"
#include "StringUtils.h"
#include "Testing.h"

#define FO_API_CRITTER_VIEW_IMPL
#include "ScriptApi.h"

PROPERTIES_IMPL(CritterView, "Critter", false);
#define FO_API_CRITTER_PROPERTY(access, type, name, ...) \
    CLASS_PROPERTY_IMPL(CritterView, access, type, name, __VA_ARGS__);
#include "ScriptApi.h"

CritterView::CritterView(uint id, ProtoCritter* proto, CritterViewSettings& sett, SpriteManager& spr_mngr,
    ResourceManager& res_mngr, EffectManager& effect_mngr, ClientScriptSystem& script_sys, GameTimer& game_time,
    bool mapper_mode) :
    Entity(id, EntityType::CritterView, PropertiesRegistrator, proto),
    settings {sett},
    geomHelper(settings),
    sprMngr {spr_mngr},
    resMngr {res_mngr},
    effectMngr {effect_mngr},
    scriptSys {script_sys},
    gameTime {game_time},
    mapperMode {mapper_mode}
{
    tickFidget = gameTime.GameTick() + GenericUtils::Random(settings.CritterFidgetTime, settings.CritterFidgetTime * 2);
    DrawEffect = effectMngr.Effects.Critter;
    auto layers = GetAnim3dLayer();
    layers.resize(LAYERS3D_COUNT);
    SetAnim3dLayer(layers);
}

CritterView::~CritterView()
{
    if (Anim3d)
        sprMngr.FreePure3dAnimation(Anim3d);
    if (Anim3dStay)
        sprMngr.FreePure3dAnimation(Anim3dStay);
}

void CritterView::Init()
{
    RefreshAnim();
    AnimateStay();

    SpriteInfo* si = sprMngr.GetSpriteInfo(SprId);
    if (si)
        textRect(0, 0, si->Width, si->Height);

    SetFade(true);
}

void CritterView::Finish()
{
    SetFade(false);
    finishingTime = FadingTick;
}

bool CritterView::IsFinishing()
{
    return finishingTime != 0;
}

bool CritterView::IsFinish()
{
    return finishingTime && gameTime.GameTick() > finishingTime;
}

void CritterView::SetFade(bool fade_up)
{
    uint tick = gameTime.GameTick();
    FadingTick = tick + FADING_PERIOD - (FadingTick > tick ? FadingTick - tick : 0);
    fadeUp = fade_up;
    fadingEnable = true;
}

uchar CritterView::GetFadeAlpha()
{
    uint tick = gameTime.GameTick();
    int fading_proc = 100 - GenericUtils::Procent(FADING_PERIOD, FadingTick > tick ? FadingTick - tick : 0);
    fading_proc = CLAMP(fading_proc, 0, 100);
    if (fading_proc >= 100)
    {
        fading_proc = 100;
        fadingEnable = false;
    }

    return fadeUp == true ? (fading_proc * 0xFF) / 100 : ((100 - fading_proc) * 0xFF) / 100;
}

void CritterView::AddItem(ItemView* item)
{
    item->SetAccessory(ITEM_ACCESSORY_CRITTER);
    item->SetCritId(Id);

    InvItems.push_back(item);

    std::sort(InvItems.begin(), InvItems.end(),
        [](ItemView* l, ItemView* r) { return l->GetSortValue() < r->GetSortValue(); });

    if (item->GetCritSlot() && !IsAnim())
        AnimateStay();
}

void CritterView::DeleteItem(ItemView* item, bool animate)
{
    item->SetAccessory(ITEM_ACCESSORY_NONE);
    item->SetCritId(0);
    item->SetCritSlot(0);

    auto it = std::find(InvItems.begin(), InvItems.end(), item);
    RUNTIME_ASSERT(it != InvItems.end());
    InvItems.erase(it);

    item->IsDestroyed = true;
    scriptSys.RemoveEntity(item);
    item->Release();

    if (animate && !IsAnim())
        AnimateStay();
}

void CritterView::DeleteAllItems()
{
    while (!InvItems.empty())
        DeleteItem(*InvItems.begin(), false);
}

ItemView* CritterView::GetItem(uint item_id)
{
    for (auto it = InvItems.begin(), end = InvItems.end(); it != end; ++it)
    {
        ItemView* item = *it;
        if (item->GetId() == item_id)
            return item;
    }
    return nullptr;
}

ItemView* CritterView::GetItemByPid(hash item_pid)
{
    for (auto it = InvItems.begin(), end = InvItems.end(); it != end; ++it)
        if ((*it)->GetProtoId() == item_pid)
            return *it;
    return nullptr;
}

ItemView* CritterView::GetItemSlot(int slot)
{
    for (auto it = InvItems.begin(), end = InvItems.end(); it != end; ++it)
        if ((*it)->GetCritSlot() == slot)
            return *it;
    return nullptr;
}

void CritterView::GetItemsSlot(int slot, ItemViewVec& items)
{
    for (auto it = InvItems.begin(), end = InvItems.end(); it != end; ++it)
    {
        ItemView* item = *it;
        if (slot == -1 || item->GetCritSlot() == slot)
            items.push_back(item);
    }
}

uint CritterView::CountItemPid(hash item_pid)
{
    uint result = 0;
    for (auto it = InvItems.begin(), end = InvItems.end(); it != end; ++it)
        if ((*it)->GetProtoId() == item_pid)
            result += (*it)->GetCount();
    return result;
}

bool CritterView::IsCombatMode()
{
    return GetTimeoutBattle() > gameTime.GetFullSecond();
}

bool CritterView::CheckFind(int find_type)
{
    if (IsNpc())
    {
        if (FLAG(find_type, FIND_ONLY_PLAYERS))
            return false;
    }
    else
    {
        if (FLAG(find_type, FIND_ONLY_NPC))
            return false;
    }
    return FLAG(find_type, FIND_ALL) || (IsLife() && FLAG(find_type, FIND_LIFE)) ||
        (IsKnockout() && FLAG(find_type, FIND_KO)) || (IsDead() && FLAG(find_type, FIND_DEAD));
}

uint CritterView::GetAttackDist()
{
    uint dist = 0;
    scriptSys.CritterGetAttackDistantionEvent(this, nullptr, 0, dist);
    return dist;
}

void CritterView::DrawStay(Rect r)
{
    if (gameTime.FrameTick() - staySprTick > 500)
    {
        staySprDir++;
        if (staySprDir >= settings.MapDirCount)
            staySprDir = 0;
        staySprTick = gameTime.FrameTick();
    }

    int dir = (!IsLife() ? GetDir() : staySprDir);
    uint anim1 = GetAnim1();
    uint anim2 = GetAnim2();

    if (!Anim3d)
    {
        AnyFrames* anim = resMngr.GetCrit2dAnim(GetModelName(), anim1, anim2, dir);
        if (anim)
        {
            uint spr_id = (IsLife() ? anim->Ind[0] : anim->Ind[anim->CntFrm - 1]);
            sprMngr.DrawSpriteSize(spr_id, r.L, r.T, r.W(), r.H(), false, true);
        }
    }
    else if (Anim3dStay)
    {
        Anim3dStay->SetDir(dir);
        Anim3dStay->SetAnimation(
            anim1, anim2, GetLayers3dData(), ANIMATION_STAY | ANIMATION_PERIOD(100) | ANIMATION_NO_SMOOTH);
        sprMngr.Draw3d(r.CX(), r.B, Anim3dStay, COLOR_IFACE);
    }
}

bool CritterView::IsLastHexes()
{
    return !LastHexX.empty() && !LastHexY.empty();
}

void CritterView::FixLastHexes()
{
    if (IsLastHexes() && LastHexX[LastHexX.size() - 1] == GetHexX() && LastHexY[LastHexY.size() - 1] == GetHexY())
        return;
    LastHexX.push_back(GetHexX());
    LastHexY.push_back(GetHexY());
}

ushort CritterView::PopLastHexX()
{
    ushort hx = LastHexX[LastHexX.size() - 1];
    LastHexX.pop_back();
    return hx;
}

ushort CritterView::PopLastHexY()
{
    ushort hy = LastHexY[LastHexY.size() - 1];
    LastHexY.pop_back();
    return hy;
}

void CritterView::Move(int dir)
{
    if (dir < 0 || dir >= settings.MapDirCount || GetIsNoRotate())
        dir = 0;

    SetDir(dir);

    uint time_move = (IsRunning ? GetRunTime() : GetWalkTime());

    TickStart(time_move);
    animStartTick = gameTime.GameTick();

    if (!Anim3d)
    {
        if (_str().parseHash(GetModelName()).startsWith("art/critters/"))
        {
            uint anim1 = (IsRunning ? ANIM1_UNARMED : GetAnim1());
            uint anim2 = (IsRunning ? ANIM2_RUN : ANIM2_WALK);
            AnyFrames* anim = resMngr.GetCrit2dAnim(GetModelName(), anim1, anim2, dir);
            if (!anim)
                anim = resMngr.CritterDefaultAnim;

            uint step;
            uint beg_spr;
            uint end_spr;
            curSpr = lastEndSpr;

            if (!IsRunning)
            {
                uint s0 = 4;
                uint s1 = 8;
                uint s2 = 0;
                uint s3 = 0;

                if ((int)curSpr == s0 - 1 && s1)
                {
                    beg_spr = s0;
                    end_spr = s1 - 1;
                    step = 2;
                }
                else if ((int)curSpr == s1 - 1 && s2)
                {
                    beg_spr = s1;
                    end_spr = s2 - 1;
                    step = 3;
                }
                else if ((int)curSpr == s2 - 1 && s3)
                {
                    beg_spr = s2;
                    end_spr = s3 - 1;
                    step = 4;
                }
                else
                {
                    beg_spr = 0;
                    end_spr = s0 - 1;
                    step = 1;
                }
            }
            else
            {
                switch (curSpr)
                {
                default:
                case 0:
                    beg_spr = 0;
                    end_spr = 1;
                    step = 1;
                    break;
                case 1:
                    beg_spr = 2;
                    end_spr = 3;
                    step = 2;
                    break;
                case 3:
                    beg_spr = 4;
                    end_spr = 6;
                    step = 3;
                    break;
                case 6:
                    beg_spr = 7;
                    end_spr = anim->CntFrm - 1;
                    step = 4;
                    break;
                }
            }

            ClearAnim();
            animSequence.push_back({anim, time_move, beg_spr, end_spr, true, 0, anim1, anim2, nullptr});
            NextAnim(false);

            for (uint i = 0; i < step; i++)
            {
                int ox, oy;
                GetWalkHexOffsets(dir, ox, oy);
                ChangeOffs(ox, oy, true);
            }
        }
        else
        {
            uint anim1 = GetAnim1();
            uint anim2 = (IsRunning ? ANIM2_RUN : ANIM2_WALK);
            if (GetIsHide())
                anim2 = (IsRunning ? ANIM2_SNEAK_RUN : ANIM2_SNEAK_WALK);

            AnyFrames* anim = resMngr.GetCrit2dAnim(GetModelName(), anim1, anim2, dir);
            if (!anim)
                anim = resMngr.CritterDefaultAnim;

            int m1 = 0;
            if (m1 <= 0)
                m1 = 5;
            int m2 = 0;
            if (m2 <= 0)
                m2 = 2;

            uint beg_spr = lastEndSpr + 1;
            uint end_spr = beg_spr + (IsRunning ? m2 : m1);

            ClearAnim();
            animSequence.push_back({anim, time_move, beg_spr, end_spr, true, dir + 1, anim1, anim2, nullptr});
            NextAnim(false);

            int ox, oy;
            GetWalkHexOffsets(dir, ox, oy);
            ChangeOffs(ox, oy, true);
        }
    }
    else
    {
        uint anim1 = GetAnim1();
        uint anim2 = (IsRunning ? ANIM2_RUN : ANIM2_WALK);
        if (GetIsHide())
            anim2 = (IsRunning ? ANIM2_SNEAK_RUN : ANIM2_SNEAK_WALK);

        Anim3d->CheckAnimation(anim1, anim2);
        Anim3d->SetDir(dir);

        ClearAnim();
        animSequence.push_back({nullptr, time_move, 0, 0, true, dir + 1, anim1, anim2, nullptr});
        NextAnim(false);

        int ox, oy;
        GetWalkHexOffsets(dir, ox, oy);
        ChangeOffs(ox, oy, true);
    }
}

void CritterView::Action(int action, int action_ext, ItemView* item, bool local_call /* = true */)
{
    scriptSys.CritterActionEvent(local_call, this, action, action_ext, item);

    switch (action)
    {
    case ACTION_KNOCKOUT:
        SetCond(COND_KNOCKOUT);
        SetAnim2Knockout(action_ext);
        break;
    case ACTION_STANDUP:
        SetCond(COND_LIFE);
        break;
    case ACTION_DEAD: {
        SetCond(COND_DEAD);
        SetAnim2Dead(action_ext);
        CritterAnim* anim = GetCurAnim();
        needReSet = true;
        reSetTick = gameTime.GameTick() + (anim && anim->Anim ? anim->Anim->Ticks : 1000);
    }
    break;
    case ACTION_CONNECT:
        UNSETFLAG(Flags, FCRIT_DISCONNECT);
        break;
    case ACTION_DISCONNECT:
        SETFLAG(Flags, FCRIT_DISCONNECT);
        break;
    case ACTION_RESPAWN:
        SetCond(COND_LIFE);
        Alpha = 0;
        SetFade(true);
        AnimateStay();
        needReSet = true;
        reSetTick = gameTime.GameTick(); // Fast
        break;
    case ACTION_REFRESH:
        if (Anim3d)
            Anim3d->StartMeshGeneration();
        break;
    default:
        break;
    }

    if (!IsAnim())
        AnimateStay();
}

void CritterView::NextAnim(bool erase_front)
{
    if (animSequence.empty())
        return;
    if (erase_front)
    {
        SAFEREL((*animSequence.begin()).ActiveItem);
        animSequence.erase(animSequence.begin());
    }
    if (animSequence.empty())
        return;

    CritterAnim& cr_anim = animSequence[0];
    animStartTick = gameTime.GameTick();

    ProcessAnim(false, !Anim3d, cr_anim.IndAnim1, cr_anim.IndAnim2, cr_anim.ActiveItem);

    if (!Anim3d)
    {
        lastEndSpr = cr_anim.EndFrm;
        curSpr = cr_anim.BeginFrm;
        SprId = cr_anim.Anim->GetSprId(curSpr);

        short ox = 0, oy = 0;
        for (int i = 0, j = curSpr % cr_anim.Anim->CntFrm; i <= j; i++)
        {
            ox += cr_anim.Anim->NextX[i];
            oy += cr_anim.Anim->NextY[i];
        }
        SetOffs(ox, oy, cr_anim.MoveText);
    }
    else
    {
        SetOffs(0, 0, cr_anim.MoveText);
        Anim3d->SetAnimation(cr_anim.IndAnim1, cr_anim.IndAnim2, GetLayers3dData(),
            (cr_anim.DirOffs ? 0 : ANIMATION_ONE_TIME) | (IsCombatMode() ? ANIMATION_COMBAT : 0));
    }
}

void CritterView::Animate(uint anim1, uint anim2, ItemView* item)
{
    uchar dir = GetDir();
    if (!anim1)
        anim1 = GetAnim1();
    if (item)
        item = item->Clone();

    if (!Anim3d)
    {
        AnyFrames* anim = resMngr.GetCrit2dAnim(GetModelName(), anim1, anim2, dir);
        if (!anim)
        {
            if (!IsAnim())
                AnimateStay();
            return;
        }

        // Todo: migrate critter on head text moving in scripts
        bool move_text = true;
        //			(Cond==COND_DEAD || Cond==COND_KNOCKOUT ||
        //			(anim2!=ANIM2_SHOW_WEAPON && anim2!=ANIM2_HIDE_WEAPON && anim2!=ANIM2_PREPARE_WEAPON &&
        // anim2!=ANIM2_TURNOFF_WEAPON && 			anim2!=ANIM2_DODGE_FRONT && anim2!=ANIM2_DODGE_BACK &&
        // anim2!=ANIM2_USE
        // &&
        // anim2!=ANIM2_PICKUP && 			anim2!=ANIM2_DAMAGE_FRONT && anim2!=ANIM2_DAMAGE_BACK && anim2!=ANIM2_IDLE
        // && anim2!=ANIM2_IDLE_COMBAT));

        animSequence.push_back({anim, anim->Ticks, 0, anim->CntFrm - 1, move_text, 0, anim->Anim1, anim->Anim2, item});
    }
    else
    {
        if (!Anim3d->CheckAnimation(anim1, anim2))
        {
            if (!IsAnim())
                AnimateStay();
            return;
        }

        animSequence.push_back({nullptr, 0, 0, 0, true, 0, anim1, anim2, item});
    }

    if (animSequence.size() == 1)
        NextAnim(false);
}

void CritterView::AnimateStay()
{
    uint anim1 = GetAnim1();
    uint anim2 = GetAnim2();

    if (!Anim3d)
    {
        AnyFrames* anim = resMngr.GetCrit2dAnim(GetModelName(), anim1, anim2, GetDir());
        if (!anim)
            anim = resMngr.CritterDefaultAnim;

        if (stayAnim.Anim != anim)
        {
            ProcessAnim(true, true, anim1, anim2, nullptr);

            stayAnim.Anim = anim;
            stayAnim.AnimTick = anim->Ticks;
            stayAnim.BeginFrm = 0;
            stayAnim.EndFrm = anim->CntFrm - 1;
            if (GetCond() == COND_DEAD)
                stayAnim.BeginFrm = stayAnim.EndFrm;
            curSpr = stayAnim.BeginFrm;
        }

        SprId = anim->GetSprId(curSpr);

        SetOffs(0, 0, true);
        short ox = 0, oy = 0;
        for (uint i = 0, j = curSpr % anim->CntFrm; i <= j; i++)
        {
            ox += anim->NextX[i];
            oy += anim->NextY[i];
        }
        ChangeOffs(ox, oy, false);
    }
    else
    {
        Anim3d->SetDir(GetDir());

        int scale_factor = GetScaleFactor();
        if (scale_factor != 0)
        {
            float scale = (float)scale_factor / 1000.0f;
            Anim3d->SetScale(scale, scale, scale);
        }

        Anim3d->CheckAnimation(anim1, anim2);

        ProcessAnim(true, false, anim1, anim2, nullptr);
        SetOffs(0, 0, true);

        if (GetCond() == COND_LIFE || GetCond() == COND_KNOCKOUT)
            Anim3d->SetAnimation(anim1, anim2, GetLayers3dData(), IsCombatMode() ? ANIMATION_COMBAT : 0);
        else
            Anim3d->SetAnimation(anim1, anim2, GetLayers3dData(),
                (ANIMATION_STAY | ANIMATION_PERIOD(100)) | (IsCombatMode() ? ANIMATION_COMBAT : 0));
    }
}

bool CritterView::IsWalkAnim()
{
    if (animSequence.size())
    {
        int anim2 = animSequence[0].IndAnim2;
        return anim2 == ANIM2_WALK || anim2 == ANIM2_RUN || anim2 == ANIM2_LIMP || anim2 == ANIM2_PANIC_RUN;
    }
    return false;
}

void CritterView::ClearAnim()
{
    for (uint i = 0, j = (uint)animSequence.size(); i < j; i++)
        SAFEREL(animSequence[i].ActiveItem);
    animSequence.clear();
}

bool CritterView::IsHaveLightSources()
{
    for (auto it = InvItems.begin(), end = InvItems.end(); it != end; ++it)
    {
        ItemView* item = *it;
        if (item->GetIsLight())
            return true;
    }
    return false;
}

bool CritterView::IsNeedReSet()
{
    return needReSet && gameTime.GameTick() >= reSetTick;
}

void CritterView::ReSetOk()
{
    needReSet = false;
}

void CritterView::TickStart(uint ms)
{
    TickCount = ms;
    StartTick = gameTime.GameTick();
}

void CritterView::TickNull()
{
    TickCount = 0;
}

bool CritterView::IsFree()
{
    return gameTime.GameTick() - StartTick >= TickCount;
}

uint CritterView::GetAnim1()
{
    switch (GetCond())
    {
    case COND_LIFE:
        return GetAnim1Life() ? GetAnim1Life() : ANIM1_UNARMED;
    case COND_KNOCKOUT:
        return GetAnim1Knockout() ? GetAnim1Knockout() : ANIM1_UNARMED;
    case COND_DEAD:
        return GetAnim1Dead() ? GetAnim1Dead() : ANIM1_UNARMED;
    default:
        break;
    }
    return ANIM1_UNARMED;
}

uint CritterView::GetAnim2()
{
    switch (GetCond())
    {
    case COND_LIFE:
        return GetAnim2Life() ? GetAnim2Life() :
                                ((IsCombatMode() && settings.Anim2CombatIdle) ? settings.Anim2CombatIdle : ANIM2_IDLE);
    case COND_KNOCKOUT:
        return GetAnim2Knockout() ? GetAnim2Knockout() : ANIM2_IDLE_PRONE_FRONT;
    case COND_DEAD:
        return GetAnim2Dead() ? GetAnim2Dead() : ANIM2_DEAD_FRONT;
    default:
        break;
    }
    return ANIM2_IDLE;
}

void CritterView::ProcessAnim(bool animate_stay, bool is2d, uint anim1, uint anim2, ItemView* item)
{
    if (is2d)
        scriptSys.Animation2dProcessEvent(animate_stay, this, anim1, anim2, item);
    else
        scriptSys.Animation3dProcessEvent(animate_stay, this, anim1, anim2, item);
}

int* CritterView::GetLayers3dData()
{
    auto layers = GetAnim3dLayer();
    RUNTIME_ASSERT(layers.size() == LAYERS3D_COUNT);
    memcpy(anim3dLayers, &layers[0], sizeof(anim3dLayers));
    return anim3dLayers;
}

bool CritterView::IsAnimAviable(uint anim1, uint anim2)
{
    if (!anim1)
        anim1 = GetAnim1();
    // 3d
    if (Anim3d)
        return Anim3d->IsAnimation(anim1, anim2);
    // 2d
    return resMngr.GetCrit2dAnim(GetModelName(), anim1, anim2, GetDir()) != nullptr;
}

void CritterView::RefreshAnim()
{
    // Release previous
    if (Anim3d)
        sprMngr.FreePure3dAnimation(Anim3d);
    Anim3d = nullptr;
    if (Anim3dStay)
        sprMngr.FreePure3dAnimation(Anim3dStay);
    Anim3dStay = nullptr;

    // Check 3d availability
    string model_name = _str().parseHash(GetModelName());
    string ext = _str(model_name).getFileExtension();
    if (!Is3dExtensionSupported(ext))
        return;

    // Try load
    sprMngr.PushAtlasType(AtlasType::Dynamic);
    Animation3d* anim3d = sprMngr.LoadPure3dAnimation(model_name, true);
    if (anim3d)
    {
        Anim3d = anim3d;
        Anim3dStay = sprMngr.LoadPure3dAnimation(model_name, false);

        Anim3d->SetDir(GetDir());
        SprId = Anim3d->SprId;

        Anim3d->SetAnimation(ANIM1_UNARMED, ANIM2_IDLE, GetLayers3dData(), 0);

        // Start mesh generation for Mapper
        if (mapperMode)
        {
            Anim3d->StartMeshGeneration();
            Anim3dStay->StartMeshGeneration();
        }
    }
    sprMngr.PopAtlasType();
}

void CritterView::ChangeDir(uchar dir, bool animate /* = true */)
{
    if (dir >= settings.MapDirCount || GetIsNoRotate())
        dir = 0;
    if (GetDir() == dir)
        return;
    SetDir(dir);
    if (Anim3d)
        Anim3d->SetDir(dir);
    if (animate && !IsAnim())
        AnimateStay();
}

void CritterView::Process()
{
    // Fading
    if (fadingEnable == true)
        Alpha = GetFadeAlpha();

    // Extra offsets
    if (OffsExtNextTick && gameTime.GameTick() >= OffsExtNextTick)
    {
        OffsExtNextTick = gameTime.GameTick() + 30;
        uint dist = GenericUtils::DistSqrt(0, 0, OxExtI, OyExtI);
        SprOx -= OxExtI;
        SprOy -= OyExtI;
        float mul = (float)(dist / 10);
        if (mul < 1.0f)
            mul = 1.0f;
        OxExtF += OxExtSpeed * mul;
        OyExtF += OyExtSpeed * mul;
        OxExtI = (int)OxExtF;
        OyExtI = (int)OyExtF;
        if (GenericUtils::DistSqrt(0, 0, OxExtI, OyExtI) > dist) // End of work
        {
            OffsExtNextTick = 0;
            OxExtI = 0;
            OyExtI = 0;
        }
        SetOffs(SprOx, SprOy, true);
    }

    // Animation
    CritterAnim& cr_anim = (animSequence.size() ? animSequence[0] : stayAnim);
    int anim_proc = (gameTime.GameTick() - animStartTick) * 100 / (cr_anim.AnimTick ? cr_anim.AnimTick : 1);
    if (anim_proc >= 100)
    {
        if (animSequence.size())
            anim_proc = 100;
        else
            anim_proc %= 100;
    }

    // Change frames
    if (!Anim3d && anim_proc < 100)
    {
        uint cur_spr = cr_anim.BeginFrm + ((cr_anim.EndFrm - cr_anim.BeginFrm + 1) * anim_proc) / 100;
        if (cur_spr != curSpr)
        {
            // Change frame
            uint old_spr = curSpr;
            curSpr = cur_spr;
            SprId = cr_anim.Anim->GetSprId(curSpr);

            // Change offsets
            short ox = 0, oy = 0;
            for (uint i = 0, j = old_spr % cr_anim.Anim->CntFrm; i <= j; i++)
            {
                ox -= cr_anim.Anim->NextX[i];
                oy -= cr_anim.Anim->NextY[i];
            }
            for (uint i = 0, j = cur_spr % cr_anim.Anim->CntFrm; i <= j; i++)
            {
                ox += cr_anim.Anim->NextX[i];
                oy += cr_anim.Anim->NextY[i];
            }
            ChangeOffs(ox, oy, cr_anim.MoveText);
        }
    }

    if (animSequence.size())
    {
        // Move offsets
        if (cr_anim.DirOffs)
        {
            int ox, oy;
            GetWalkHexOffsets(cr_anim.DirOffs - 1, ox, oy);
            SetOffs(ox - (ox * anim_proc / 100), oy - (oy * anim_proc / 100), true);

            if (anim_proc >= 100)
            {
                NextAnim(true);
                if (MoveSteps.size())
                    return;
            }
        }
        else
        {
            if (!Anim3d)
            {
                if (anim_proc >= 100)
                    NextAnim(true);
            }
            else
            {
                if (!Anim3d->IsAnimationPlaying())
                    NextAnim(true);
            }
        }

        if (MoveSteps.size())
            return;
        if (!animSequence.size())
            AnimateStay();
    }

    // Battle 3d mode
    // Todo: do same for 2d animations
    if (Anim3d && settings.Anim2CombatIdle && !animSequence.size() && GetCond() == COND_LIFE && !GetAnim2Life())
    {
        if (settings.Anim2CombatBegin && IsCombatMode() && Anim3d->GetAnim2() != (int)settings.Anim2CombatIdle)
            Animate(0, settings.Anim2CombatBegin, nullptr);
        else if (settings.Anim2CombatEnd && !IsCombatMode() && Anim3d->GetAnim2() == (int)settings.Anim2CombatIdle)
            Animate(0, settings.Anim2CombatEnd, nullptr);
    }

    // Fidget animation
    if (gameTime.GameTick() >= tickFidget)
    {
        if (!animSequence.size() && GetCond() == COND_LIFE && IsFree() && !MoveSteps.size() && !IsCombatMode())
            Action(ACTION_FIDGET, 0, nullptr);
        tickFidget =
            gameTime.GameTick() + GenericUtils::Random(settings.CritterFidgetTime, settings.CritterFidgetTime * 2);
    }
}

void CritterView::ChangeOffs(short change_ox, short change_oy, bool move_text)
{
    SetOffs(SprOx - OxExtI + change_ox, SprOy - OyExtI + change_oy, move_text);
}

void CritterView::SetOffs(short set_ox, short set_oy, bool move_text)
{
    SprOx = set_ox + OxExtI;
    SprOy = set_oy + OyExtI;
    if (SprDrawValid)
    {
        sprMngr.GetDrawRect(SprDraw, DRect);
        if (move_text)
        {
            textRect = DRect;
            if (Anim3d)
                textRect.T += sprMngr.GetSpriteInfo(SprId)->Height / 6;
        }
        if (IsChosen())
            sprMngr.SetEgg(GetHexX(), GetHexY(), SprDraw);
    }
}

void CritterView::SetSprRect()
{
    if (SprDrawValid)
    {
        Rect old = DRect;
        sprMngr.GetDrawRect(SprDraw, DRect);
        textRect.L += DRect.L - old.L;
        textRect.R += DRect.L - old.L;
        textRect.T += DRect.T - old.T;
        textRect.B += DRect.T - old.T;

        if (IsChosen())
            sprMngr.SetEgg(GetHexX(), GetHexY(), SprDraw);
    }
}

Rect CritterView::GetTextRect()
{
    if (SprDrawValid)
        return textRect;
    return Rect();
}

void CritterView::AddOffsExt(short ox, short oy)
{
    SprOx -= OxExtI;
    SprOy -= OyExtI;
    ox += OxExtI;
    oy += OyExtI;
    OxExtI = ox;
    OyExtI = oy;
    OxExtF = (float)ox;
    OyExtF = (float)oy;
    GenericUtils::GetStepsXY(OxExtSpeed, OyExtSpeed, 0, 0, ox, oy);
    OxExtSpeed = -OxExtSpeed;
    OyExtSpeed = -OyExtSpeed;
    OffsExtNextTick = gameTime.GameTick() + 30;
    SetOffs(SprOx, SprOy, true);
}

void CritterView::GetWalkHexOffsets(int dir, int& ox, int& oy)
{
    int hx = 1, hy = 1;
    geomHelper.MoveHexByDirUnsafe(hx, hy, dir);
    geomHelper.GetHexInterval(hx, hy, 1, 1, ox, oy);
}

void CritterView::SetText(const char* str, uint color, uint text_delay)
{
    tickStartText = gameTime.GameTick();
    strTextOnHead = str;
    tickTextDelay = text_delay;
    textOnHeadColor = color;
}

void CritterView::GetNameTextInfo(bool& nameVisible, int& x, int& y, int& w, int& h, int& lines)
{
    nameVisible = false;

    string str;
    if (strTextOnHead.empty())
    {
        if (IsPlayer() && !settings.ShowPlayerNames)
            return;
        if (IsNpc() && !settings.ShowNpcNames)
            return;

        nameVisible = true;

        str = (NameOnHead.empty() ? Name : NameOnHead);
        if (settings.ShowCritId)
            str += _str("  {}", GetId());
        if (FLAG(Flags, FCRIT_DISCONNECT))
            str += settings.PlayerOffAppendix;
    }
    else
    {
        str = strTextOnHead;
    }

    Rect tr = GetTextRect();
    x = (int)((float)(tr.L + tr.W() / 2 + settings.ScrOx) / settings.SpritesZoom - 100.0f);
    y = (int)((float)(tr.T + settings.ScrOy) / settings.SpritesZoom - 70.0f);

    sprMngr.GetTextInfo(200, 70, str.c_str(), -1, FT_CENTERX | FT_BOTTOM | FT_BORDERED, w, h, lines);
    x += 100 - (w / 2);
    y += 70 - h;
}

void CritterView::DrawTextOnHead()
{
    if (strTextOnHead.empty())
    {
        if (IsPlayer() && !settings.ShowPlayerNames)
            return;
        if (IsNpc() && !settings.ShowNpcNames)
            return;
    }

    if (SprDrawValid)
    {
        Rect tr = GetTextRect();
        int x = (int)((float)(tr.L + tr.W() / 2 + settings.ScrOx) / settings.SpritesZoom - 100.0f);
        int y = (int)((float)(tr.T + settings.ScrOy) / settings.SpritesZoom - 70.0f);
        Rect r(x, y, x + 200, y + 70);

        string str;
        uint color;
        if (strTextOnHead.empty())
        {
            str = (NameOnHead.empty() ? Name : NameOnHead);
            if (settings.ShowCritId)
                str += _str(" ({})", GetId());
            if (FLAG(Flags, FCRIT_DISCONNECT))
                str += settings.PlayerOffAppendix;
            color = (NameColor ? NameColor : COLOR_CRITTER_NAME);
        }
        else
        {
            str = strTextOnHead;
            color = textOnHeadColor;

            if (tickTextDelay > 500)
            {
                uint dt = gameTime.GameTick() - tickStartText;
                uint hide = tickTextDelay - 200;
                if (dt >= hide)
                {
                    uint alpha = 0xFF * (100 - GenericUtils::Procent(tickTextDelay - hide, dt - hide)) / 100;
                    color = (alpha << 24) | (color & 0xFFFFFF);
                }
            }
        }

        if (fadingEnable)
        {
            uint alpha = GetFadeAlpha();
            sprMngr.DrawStr(r, str, FT_CENTERX | FT_BOTTOM | FT_BORDERED, (alpha << 24) | (color & 0xFFFFFF));
        }
        else if (!IsFinishing())
        {
            sprMngr.DrawStr(r, str, FT_CENTERX | FT_BOTTOM | FT_BORDERED, color);
        }
    }

    if (gameTime.GameTick() - tickStartText >= tickTextDelay && !strTextOnHead.empty())
        strTextOnHead = "";
}
