#include "cbase.h"

#include "mom_run_safeguards.h"
#include "in_buttons.h"
#include "mom_player_shared.h"
#include "fmtstr.h"

#ifdef CLIENT_DLL
#include "input.h"
#include "gameui/BaseMenuPanel.h"
#else
#include "mom_modulecomms.h"
#endif

#include "tier0/memdbgon.h"

static MAKE_CONVAR(mom_run_safeguard_doublepress_maxtime, "0.5", FCVAR_ARCHIVE | FCVAR_REPLICATED,
    "Controls the max amount of time (in seconds) that a double press is registered for run safeguards "
    "with the doublepress safeguard mode active.\n",
    0.0f, 5.0f);

static MAKE_CONVAR(mom_run_safeguard_warning_delay, "10.0", FCVAR_ARCHIVE | FCVAR_REPLICATED,
    "Controls the amount of time (in seconds) between warning the player that the desired command is safeguarded.\n", 0.0f, 30.0f);

static MAKE_CONVAR(mom_run_safeguard_practicemode, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED,
    "Changes the safeguard for enabling practice mode during a run.\n"
    "0 = OFF,\n"
    "1 = Enable when not pressing any movement keys,\n"
    "2 - Enable on double press.\n",
    RUN_SAFEGUARD_MODE_NONE, RUN_SAFEGUARD_MODE_DOUBLEPRESS);

static MAKE_CONVAR(mom_run_safeguard_restart, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED,
    "Changes the safeguard for restarting the map during a run.\n"
    "0 = OFF,\n"
    "1 = Enable when not pressing any movement keys,\n"
    "2 - Enable on double press.\n",
    RUN_SAFEGUARD_MODE_NONE, RUN_SAFEGUARD_MODE_DOUBLEPRESS);

static MAKE_CONVAR(mom_run_safeguard_saveloc_tele, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED,
    "Changes the safeguard for teleporting to a saved location during a run.\n"
    "0 = OFF,\n"
    "1 = Enable when not pressing any movement keys,\n"
    "2 - Enable on double press.\n",
    RUN_SAFEGUARD_MODE_NONE, RUN_SAFEGUARD_MODE_DOUBLEPRESS);

static MAKE_CONVAR(mom_run_safeguard_chat_open, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED,
    "Changes the safeguard for opening chat during a run.\n"
    "0 = OFF,\n"
    "1 = Enable when not pressing any movement keys,\n"
    "2 - Enable on double press.\n",
    RUN_SAFEGUARD_MODE_NONE, RUN_SAFEGUARD_MODE_DOUBLEPRESS);
    
static MAKE_CONVAR(mom_run_safeguard_restart_stage, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED,
    "Changes the safeguard for teleporting to a stage/restarting the current stage during a run.\n"
    "0 = OFF,\n"
    "1 = Enable when not pressing any movement keys,\n"
    "2 - Enable on double press.\n",
    RUN_SAFEGUARD_MODE_NONE, RUN_SAFEGUARD_MODE_DOUBLEPRESS);

static MAKE_TOGGLE_CONVAR(mom_run_safeguard_change_map, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Changes the safeguard setting for preventing changing map while in a run. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_run_safeguard_quit_map, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Changes the safeguard setting for preventing quitting/disconnecting while in a run. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_run_safeguard_quit_game, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Changes the safeguard setting for preventing quitting the game while in a run. 0 = OFF, 1 = ON\n");

CRunSafeguard::CRunSafeguard(const char *szAction)
    : m_flLastTimePressed(0.0f), m_flLastTimeWarned(0.0f), m_pRelatedVar(nullptr), m_bIgnoredInMenu(true)
{
    Q_strncpy(m_szAction, szAction, sizeof(m_szAction));
}

void CRunSafeguard::Reset()
{
    m_flLastTimePressed = -(mom_run_safeguard_doublepress_maxtime.GetFloat());
    m_flLastTimeWarned = -(mom_run_safeguard_warning_delay.GetFloat());
}

bool CRunSafeguard::IsSafeguarded(RunSafeguardMode_t mode)
{
    CMomentumPlayer *pPlayer;
#ifdef CLIENT_DLL
    pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
#elif GAME_DLL
    pPlayer = CMomentumPlayer::GetLocalPlayer();
#endif
    if (!pPlayer)
        return false;

    CMomRunEntityData *pEntData = pPlayer->GetRunEntData();

    if (!pEntData)
        return false;

    if (pEntData->m_iTimerState != TIMER_STATE_RUNNING || pPlayer->IsObserver())
        return false;

#ifdef GAME_DLL
    if (m_bIgnoredInMenu && g_pRunSafeguards->IsGameUIActive())
        return false;
#else
    if (m_bIgnoredInMenu && g_pBasePanel->GetMainMenu()->IsVisible())
        return false;
#endif

    int nButtons = 
#ifdef CLIENT_DLL
    ::input->GetButtonBits(engine->IsPlayingDemo());
#else
    pPlayer->m_nButtons;
#endif

    switch (mode)
    {
    case RUN_SAFEGUARD_MODE_MOVEMENTKEYS:
        return IsMovementKeysSafeguarded(nButtons, pPlayer);
    case RUN_SAFEGUARD_MODE_DOUBLEPRESS:
        return IsDoublePressSafeguarded(pPlayer);
    case RUN_SAFEGUARD_MODE_POPUP_CONFIRM:
        return m_pRelatedVar && m_pRelatedVar->GetBool();
    default:
        return false;
    } 
}

bool CRunSafeguard::IsMovementKeysSafeguarded(int nButtons, CBasePlayer *pPlayer)
{
    if ((nButtons & (IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT | IN_BACK | IN_JUMP | IN_DUCK | IN_WALK)) != 0)
    {
        if (gpGlobals->curtime - m_flLastTimeWarned > mom_run_safeguard_warning_delay.GetFloat())
        {
            ClientPrint(pPlayer, HUD_PRINTTALK, CFmtStr("You cannot %s when movement keys are held down while the timer is running! You can turn this safeguard off in the Gameplay Settings!\n", m_szAction));
            m_flLastTimeWarned = gpGlobals->curtime;
        }

        return true;
    }
    return false;
}

bool CRunSafeguard::IsDoublePressSafeguarded(CBasePlayer *pPlayer)
{
    if (gpGlobals->curtime - m_flLastTimePressed > mom_run_safeguard_doublepress_maxtime.GetFloat())
    {
        if (gpGlobals->curtime - m_flLastTimeWarned > mom_run_safeguard_warning_delay.GetFloat())
        {
            ClientPrint(pPlayer, HUD_PRINTTALK, CFmtStr("You must double press the key to %s while the timer is running! You can turn this safeguard off in the Gameplay Settings!\n", m_szAction));
            m_flLastTimeWarned = gpGlobals->curtime;
        }

        m_flLastTimePressed = gpGlobals->curtime;
        return true;
    }

    Reset();
    return false;
}

MomRunSafeguards::MomRunSafeguards()
{
    m_pSafeguards[RUN_SAFEGUARD_PRACTICEMODE] = new CRunSafeguard("enable practice mode");
    m_pSafeguards[RUN_SAFEGUARD_RESTART] = new CRunSafeguard("restart map");
    m_pSafeguards[RUN_SAFEGUARD_SAVELOC_TELE] = new CRunSafeguard("teleport to saveloc");
    m_pSafeguards[RUN_SAFEGUARD_CHAT_OPEN] = new CRunSafeguard("open chat");
    m_pSafeguards[RUN_SAFEGUARD_RESTART_STAGE] = new CRunSafeguard("teleport to a stage");

    m_pSafeguards[RUN_SAFEGUARD_MAP_CHANGE] = new CRunSafeguard("changing map");
    m_pSafeguards[RUN_SAFEGUARD_MAP_CHANGE]->SetRelevantCVar(&mom_run_safeguard_change_map);
    m_pSafeguards[RUN_SAFEGUARD_MAP_CHANGE]->SetIgnoredInMenu(false);

    m_pSafeguards[RUN_SAFEGUARD_QUIT_TO_MENU] = new CRunSafeguard("quitting to menu");
    m_pSafeguards[RUN_SAFEGUARD_QUIT_TO_MENU]->SetRelevantCVar(&mom_run_safeguard_quit_map);
    m_pSafeguards[RUN_SAFEGUARD_QUIT_TO_MENU]->SetIgnoredInMenu(false);

    m_pSafeguards[RUN_SAFEGUARD_QUIT_GAME] = new CRunSafeguard("quitting the game");
    m_pSafeguards[RUN_SAFEGUARD_QUIT_GAME]->SetRelevantCVar(&mom_run_safeguard_quit_game);
    m_pSafeguards[RUN_SAFEGUARD_QUIT_GAME]->SetIgnoredInMenu(false);

#ifdef GAME_DLL
    m_bGameUIActive = false;
    g_pModuleComms->ListenForEvent("gameui_toggle", UtlMakeDelegate(this, &MomRunSafeguards::OnGameUIToggled));
#endif
}

void MomRunSafeguards::ResetAllSafeguards()
{
    for (int i = 0; i < RUN_SAFEGUARD_COUNT; i++)
    {
        m_pSafeguards[i]->Reset();
    }
}

void MomRunSafeguards::ResetSafeguard(int type)
{
    if (type <= RUN_SAFEGUARD_INVALID || type >= RUN_SAFEGUARD_COUNT)
        return;

    ResetSafeguard(RunSafeguardType_t(type));
}

void MomRunSafeguards::ResetSafeguard(RunSafeguardType_t type)
{
    m_pSafeguards[type]->Reset();
}

#ifdef GAME_DLL
void MomRunSafeguards::OnGameUIToggled(KeyValues *pKv)
{
    m_bGameUIActive = pKv->GetBool("activated");
}
#endif

bool MomRunSafeguards::IsSafeguarded(RunSafeguardType_t type)
{
    return IsSafeguarded(type, GetModeFromType(type));
}

bool MomRunSafeguards::IsSafeguarded(RunSafeguardType_t type, RunSafeguardMode_t mode)
{
    return m_pSafeguards[type]->IsSafeguarded(mode);
}

RunSafeguardMode_t MomRunSafeguards::GetModeFromType(int type)
{
    return (type > RUN_SAFEGUARD_INVALID && type < RUN_SAFEGUARD_COUNT) ?
           GetModeFromType(RunSafeguardType_t(type)) :
           RUN_SAFEGUARD_MODE_INVALID;
}

RunSafeguardMode_t MomRunSafeguards::GetModeFromType(RunSafeguardType_t type)
{
    switch (type)
    {
    case RUN_SAFEGUARD_PRACTICEMODE:
        return RunSafeguardMode_t(mom_run_safeguard_practicemode.GetInt());
    case RUN_SAFEGUARD_RESTART:
        return RunSafeguardMode_t(mom_run_safeguard_restart.GetInt());
    case RUN_SAFEGUARD_SAVELOC_TELE:
        return RunSafeguardMode_t(mom_run_safeguard_saveloc_tele.GetInt());
    case RUN_SAFEGUARD_CHAT_OPEN:
        return RunSafeguardMode_t(mom_run_safeguard_chat_open.GetInt());
    case RUN_SAFEGUARD_RESTART_STAGE:
        return RunSafeguardMode_t(mom_run_safeguard_restart_stage.GetInt());
    case RUN_SAFEGUARD_QUIT_GAME:
    case RUN_SAFEGUARD_QUIT_TO_MENU:
    case RUN_SAFEGUARD_MAP_CHANGE:
        return RUN_SAFEGUARD_MODE_POPUP_CONFIRM;
    default:
        return RUN_SAFEGUARD_MODE_INVALID;
    }
}

void MomRunSafeguards::SetMode(int type, int mode)
{
    if (type <= RUN_SAFEGUARD_INVALID || type >= RUN_SAFEGUARD_COUNT)
        return;

    if (mode > RUN_SAFEGUARD_MODE_INVALID && mode < RUN_SAFEGUARD_MODE_COUNT)
    {
        SetMode(RunSafeguardType_t(type), RunSafeguardMode_t(mode));
    }
    else
    {
        SetMode(RunSafeguardType_t(type), GetModeFromType(type));
    }
}

void MomRunSafeguards::SetMode(RunSafeguardType_t type, RunSafeguardMode_t mode)
{
    switch (type)
    {
    case RUN_SAFEGUARD_PRACTICEMODE:
        mom_run_safeguard_practicemode.SetValue(mode);
        break;
    case RUN_SAFEGUARD_RESTART:
        mom_run_safeguard_restart.SetValue(mode);
        break;
    case RUN_SAFEGUARD_SAVELOC_TELE:
        mom_run_safeguard_saveloc_tele.SetValue(mode);
        break;
    case RUN_SAFEGUARD_CHAT_OPEN:
        mom_run_safeguard_chat_open.SetValue(mode);
        break;
    case RUN_SAFEGUARD_RESTART_STAGE:
        mom_run_safeguard_restart_stage.SetValue(mode);
        break;
    }
}

static MomRunSafeguards s_RunSafeguards;
MomRunSafeguards *g_pRunSafeguards = &s_RunSafeguards;