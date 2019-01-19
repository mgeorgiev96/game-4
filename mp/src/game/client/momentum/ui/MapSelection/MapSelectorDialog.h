#pragma once

#include "vgui_controls/Frame.h"

class CMapContextMenu;
class CDialogMapInfo;
class CLocalMaps;
class COnlineMaps;
class IMapList;
class MapFilterPanel;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CMapSelectorDialog : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(CMapSelectorDialog, vgui::Frame);

public:
    // Construction/destruction
    CMapSelectorDialog(vgui::VPANEL parent);
    ~CMapSelectorDialog(void);

    void		Initialize(void);

    // displays the dialog, moves it into focus, updates if it has to
    void		Open(void);
    
    void OnClose() OVERRIDE;

    // called every frame
    virtual void OnTick() OVERRIDE;

    // updates status text at bottom of window
    void UpdateStatusText(const char *format, ...);

    // context menu access
    CMapContextMenu *GetContextMenu(Panel *pParent);

    // returns a pointer to a static instance of this dialog
    // valid for use only in sort functions
    static CMapSelectorDialog *GetInstance();

    // opens a game info dialog from a game list
    CDialogMapInfo *OpenMapInfoDialog(IMapList *gameList, KeyValues *pMap);

    // opens a game info dialog by a specified IP, not attached to any game list
    CDialogMapInfo *OpenMapInfoDialog(int serverIP, uint16 connPort, uint16 queryPort);

    // closes all the game info dialogs
    void CloseAllMapInfoDialogs();
    CDialogMapInfo *GetDialogGameInfoForFriend(uint64 ulSteamIDFriend);

    // accessor to the filter save data
    KeyValues *GetCurrentTabFilterData();
    void LoadTabFilterData(const char *pTabName);
    void ApplyFiltersToCurrentTab();

    // load/saves filter & favorites settings from disk
    void		LoadUserData();
    void		SaveUserData();

    // forces the currently active page to refresh
    void		RefreshCurrentPage();

    void OnSizeChanged(int wide, int tall) OVERRIDE;

private:

    // current game list change
    MESSAGE_FUNC(OnTabChanged, "PageChanged");

    // notification that we connected / disconnected
    MESSAGE_FUNC_PARAMS(OnConnectToGame, "ConnectedToGame", kv);
    MESSAGE_FUNC(OnDisconnectFromGame, "DisconnectedFromGame");

    virtual void ActivateBuildMode();

private:
    // list of all open game info dialogs
    CUtlVector<vgui::DHANDLE<CDialogMapInfo> > m_vecMapInfoDialogs;

    // pointer to current game list
    IMapList *m_pCurrentMapList;

    // Status text
    vgui::Label	*m_pStatusLabel;

    // property sheet
    vgui::PropertySheet *m_pTabPanel;

    //Map tabs
    CLocalMaps *m_pLibraryMaps;
    COnlineMaps *m_pOnline;

    // Filters
    MapFilterPanel *m_pFilterPanel;

    //Filter data
    KeyValues *m_pSavedData;//Saved on disk filter data
    KeyValues *m_pFilterData;//Current filter data in the Dialog

    // context menu
    CMapContextMenu *m_pContextMenu;

    // currently connected game
    bool m_bCurrentlyConnected;/*
    gameserveritem_t m_CurrentConnection;*/
};

// singleton accessor
extern CMapSelectorDialog &MapSelectorDialog();