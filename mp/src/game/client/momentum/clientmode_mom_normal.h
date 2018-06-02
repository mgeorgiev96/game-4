//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#if !defined(CLIENTMODE_MOM_NORM_H)
#define CLIENTMODE_MOM_NORM_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/Cursor.h>
#include <vgui_controls/EditablePanel.h>
#include "ClientTimesDisplay.h"
#include "clientmode_shared.h"
#include "hud_mapfinished.h"
#include "hud_menu_static.h"
#include "momSpectatorGUI.h"

class CHudViewport;

namespace vgui
{
typedef unsigned long HScheme;
}

enum
{
    MD_NONE = 0,
    MD_Forwards,
    MD_Sideways,
    MD_Sideways2,
    MD_Backwards,
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class ClientModeMOMNormal : public ClientModeShared
{
  public:
    DECLARE_CLASS(ClientModeMOMNormal, ClientModeShared);

    ClientModeMOMNormal();
    ~ClientModeMOMNormal();

    void Init() OVERRIDE;
    bool ShouldDrawCrosshair(void) OVERRIDE;
    // NOTE: This includes mouse inputs!!!
    int HudElementKeyInput(int down, ButtonCode_t keynum, const char *pszCurrentBinding) OVERRIDE;
    int HandleSpectatorKeyInput(int down, ButtonCode_t keynum, const char *pszCurrentBinding) OVERRIDE;

    bool DoPostScreenSpaceEffects(const CViewSetup* pSetup) OVERRIDE;

    void SetupPointers();
    int MovementDirection(const QAngle viewangles, const Vector velocity);
    bool CreateMove(float flInputSampleTime, CUserCmd *cmd);

  public:
    CHudMenuStatic *m_pHudMenuStatic;
    CHudMapFinishedDialog *m_pHudMapFinished;
    CClientTimesDisplay *m_pLeaderboards;
    CMOMSpectatorGUI *m_pSpectatorGUI;
};

extern IClientMode *GetClientModeNormal();
extern vgui::HScheme g_hVGuiCombineScheme;

#endif // CLIENTMODE_MOM_NORMAL
