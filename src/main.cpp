/*
 * **==============================================================================
 * **
 * ** Copyright (c) Microsoft Corporation. All rights reserved. See file LICENSE
 * ** for license information.
 * **
 * **==============================================================================
 * */
#include <iostream>
#include <fstream>
#include <sstream>
#if VC_SUPPORTS_STD_FILESYSTEM
#include <filesystem> Not yet supported in vc
#endif
#include "windows.h"
#include "service_unit.h"
#include <map>

char usage[] = "usage: "\
               "  systemctl start <servicename>\n" \
               "  systemctl start <servicename>\n";

using namespace std;

wstring SystemDUnitPool::UNIT_DIRECTORY_PATH = L""; // Quasi constant
wstring SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH = L""; // Quasi constant
wstring SystemDUnitPool::UNIT_WORKING_DIRECTORY_PATH = L""; // Quasi constant

enum SystemCtl_Cmd {
    SYETEMCTL_CMD_DAEMON_RELOAD = 0,
    SYSTEMCTL_CMD_LIST_UNITS,
    SYSTEMCTL_CMD_LIST_SOCKETS,
    SYSTEMCTL_CMD_LIST_TIMERS,
    SYSTEMCTL_CMD_START,
    SYSTEMCTL_CMD_STOP,
    SYSTEMCTL_CMD_RELOAD,
    SYSTEMCTL_CMD_RESTART,
    SYSTEMCTL_CMD_TRY_RESTART,
    SYSTEMCTL_CMD_RELOAD_OR_RESTART,
    SYSTEMCTL_CMD_TRY_RELOAD_OR_RESTART,
    SYSTEMCTL_CMD_ISOLATE,
    SYSTEMCTL_CMD_KILL,
    SYSTEMCTL_CMD_IS_ACTIVE,
    SYSTEMCTL_CMD_IS_FAILED,
    SYSTEMCTL_CMD_STATUS,
    SYSTEMCTL_CMD_SHOW,
    SYSTEMCTL_CMD_CAT,
    SYSTEMCTL_CMD_SET_PROPERTY,
    SYSTEMCTL_CMD_HELP,
    SYSTEMCTL_CMD_RESET_FAILED,
    SYSTEMCTL_CMD_LIST_DEPENDENCIES,
    SYSTEMCTL_CMD_LIST_UNIT_FILES,
    SYSTEMCTL_CMD_ENABLE,
    SYSTEMCTL_CMD_DISABLE,
    SYSTEMCTL_CMD_REENABLE,
    SYSTEMCTL_CMD_PRESET,
    SYSTEMCTL_CMD_PRESET_ALL,
    SYSTEMCTL_CMD_IS_ENABLED,
    SYSTEMCTL_CMD_MASK,
    SYSTEMCTL_CMD_UNMASK,
    SYSTEMCTL_CMD_LINK,
    SYSTEMCTL_CMD_REVERT,
    SYSTEMCTL_CMD_ADD_WANTS,
    SYSTEMCTL_CMD_ADD_REQUIRES,
    SYSTEMCTL_CMD_EDIT,
    SYSTEMCTL_CMD_GET_DEFAULT,
    SYSTEMCTL_CMD_SET_DEFAULT,
    SYSTEMCTL_CMD_LIST_MACHINES,
    SYSTEMCTL_CMD_LIST_JOBS,
    SYSTEMCTL_CMD_CANCEL,
    SYSTEMCTL_CMD_SHOW_ENVIRONMENT,
    SYSTEMCTL_CMD_SET_ENVIRONMENT,
    SYSTEMCTL_CMD_UNSET_ENVIRONMENT,
    SYSTEMCTL_CMD_IMPORT_ENVIRONMENT,
    SYSTEMCTL_CMD_DAEMON_RELOAD,
    SYSTEMCTL_CMD_DAEMON_REEXEC,
    SYSTEMCTL_CMD_IS_SYSTEM_RUNNING,
    SYSTEMCTL_CMD_DEFAULT,
    SYSTEMCTL_CMD_RESCUE,
    SYSTEMCTL_CMD_EMERGENCY,
    SYSTEMCTL_CMD_HALT,
    SYSTEMCTL_CMD_POWEROFF,
    SYSTEMCTL_CMD_REBOOT,
    SYSTEMCTL_CMD_KEXEC,
    SYSTEMCTL_CMD_EXIT,
    SYSTEMCTL_CMD_SWITCH_ROOT,
    SYSTEMCTL_CMD_SUSPEND,
    SYSTEMCTL_CMD_HIBERNATE,
    SYSTEMCTL_CMD_HYBRID_SLEEP
};


int SystemCtrl_Cmd_List_Units( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_List_Sockets( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_List_Timers( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Start( int argc, wchar_t *argv[] )
{
   wstring usage = L"usage: Systemctl start <unitname>[ unitname .. unitname]\n";

    wstring unitname = argv[2] ;
    if (unitname.empty()) {
         // Complain and exit
         wcerr << "No unit specified\n";
         wcerr << usage.c_str();
         exit(1);
    }

    if (unitname.rfind(L".service") == string::npos &&
        unitname.rfind(L".target")  == string::npos &&
        unitname.rfind(L".timer")   == string::npos &&
        unitname.rfind(L".socket")  == string::npos ) {
          unitname.append(L".service");
    }

    class SystemDUnit *unit = SystemDUnitPool::FindUnit(unitname);
    if (!unit) {
        // Complain and exit
        wcerr << "Failed to enable unit: Unit file " << unitname.c_str() << "does not exist\n";
        wcerr << usage.c_str();
        exit(1);
    }
    unit->StartService(false); // We will add non-blocking later
    return 0;
}


int SystemCtrl_Cmd_Stop( int argc, wchar_t *argv[] )
{
   wstring usage = L"usage: Systemctl stop <unitname>\n";

    wstring unitname = argv[2] ;
    if (unitname.empty()) {
         // Complain and exit
         wcerr << "No unit specified\n";
         wcerr << usage.c_str();
         exit(1);
    }

    if (unitname.rfind(L".service") == string::npos) {
          unitname.append(L".service");
    }

    class SystemDUnit *unit = SystemDUnitPool::FindUnit(unitname);
    if (!unit) {
        // Complain and exit
        wcerr << "Failed to enable unit: Unit file " << unitname.c_str() << "does not exist\n";
        wcerr << usage.c_str();
        exit(1);
    }
    unit->StopService(false); // We will add non-blocking later
    return 0;
}


int SystemCtrl_Cmd_Reload( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Restart( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Try_Restart( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Reload_Or_Restart( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Try_Reload_Or_Restart( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Isolate( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Kill( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Is_Active( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Is_Failed( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Status( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Show( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Cat( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Set_Property( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Help( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Reset_Failed( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_List_Dependencies( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_List_Unit_Files( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Enable( int argc, wchar_t *argv[] )
{
    wstring usage = L"usage: Systemctl enable <unitname>\n";

    wstring unitname = argv[2] ;
    if (unitname.empty()) {
         // Complain and exit
         wcerr << "No unit specified\n";
         wcerr << usage.c_str();
         exit(1);
    }


    // We allow a shorthand reference via just the service name, but 
    // we recognise the valid file extensions if given.
    if (unitname.rfind(L".service") == string::npos &&
        unitname.rfind(L".target")  == string::npos &&
        unitname.rfind(L".timer")   == string::npos &&
        unitname.rfind(L".socket")  == string::npos ) {
          unitname.append(L".service");
    }

    wstring service_unit_path = SystemDUnitPool::UNIT_DIRECTORY_PATH+L"\\"+unitname; // We only look for service unit files in the top level directory
    class SystemDUnit *unit = SystemDUnitPool::FindUnit(unitname);
    if (!unit) {
        unit = SystemDUnitPool::ReadServiceUnit(unitname, service_unit_path);
        if (!unit) {
            // Complain and exit
            wcerr << "Failed to load unit: Unit file " << service_unit_path.c_str() << "is invalid\n";
            return false;       
       }
    }
    unit->Enable(false); // We will add non-blocking later

    return 0;
}


int SystemCtrl_Cmd_Disable( int argc, wchar_t *argv[] )
{
    wstring usage = L"usage: Systemctl disable <unitname>\n";

    wstring unitname = argv[3] ;
    if (unitname.empty()) {
         // Complain and exit
         wcerr << "No unit specified\n";
         wcerr << usage.c_str();
         exit(1);
    }

    if (unitname.rfind(L".service") == string::npos) {
          unitname.append(L".service");
    }

    class SystemDUnit *unit = SystemDUnitPool::FindUnit(unitname);
    if (!unit) {
        // Complain and exit
        wcerr << "Failed to enable unit: Unit file " << unitname.c_str() << "does not exist\n";
        wcerr << usage.c_str();
        exit(1);
    }
    unit->Disable(false); // We will add non-blocking later
    return 0;
}


int SystemCtrl_Cmd_Reenable( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Preset( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Preset_All( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Is_Enabled( int argc, wchar_t *argv[] )
{
    wstring unitname = argv[2];
    if (unitname.rfind(L".service") == string::npos) {
          unitname.append(L".service");
    }

    SystemDUnit *unit = SystemDUnitPool::FindUnit(unitname);
    if (!unit) {
            // Complain and exit
            wcerr << "Failed to get unit file state for " << unitname.c_str() << ": No such file or directory\n";
            exit(1);
    }
    if (unit->IsEnabled() ) {
        wcout << L"enabled";
        exit(0);
    }
    else {
        wcout << L"false";
        exit(1);
    }
    return 0;
}


int SystemCtrl_Cmd_Mask( int argc, wchar_t *argv[] )
{
    wstring usage = L"usage: Systemctl mask <unitname>\n";

    wstring unitname = argv[3] ;
    if (unitname.empty()) {
         // Complain and exit
         wcerr << "No unit specified\n";
         wcerr << usage.c_str();
         exit(1);
    }

    if (unitname.rfind(L".service") == string::npos) {
          unitname.append(L".service");
    }

    class SystemDUnit *unit = SystemDUnitPool::FindUnit(unitname);
    if (!unit) {
        // Complain and exit
        wcerr << "Failed to enable unit: Unit file " << unitname.c_str() << "does not exist\n";
        wcerr << usage.c_str();
        exit(1);
    }
    unit->Mask(false); // We will add non-blocking later
    return 0;
}


int SystemCtrl_Cmd_Unmask( int argc, wchar_t *argv[] )
{
    wstring usage = L"usage: Systemctl unmask <unitname>\n";

    wstring unitname = argv[3] ;
    if (unitname.empty()) {
         // Complain and exit
         wcerr << "No unit specified\n";
         wcerr << usage.c_str();
         exit(1);
    }

    if (unitname.rfind(L".service") == string::npos) {
          unitname.append(L".service");
    }

    class SystemDUnit *unit = SystemDUnitPool::FindUnit(unitname);
    if (!unit) {
        // Complain and exit
        wcerr << "Failed to enable unit: Unit file " << unitname.c_str() << "does not exist\n";
        wcerr << usage.c_str();
        exit(1);
    }
    unit->Unmask(false); // We will add non-blocking later
    return 0;
}


int SystemCtrl_Cmd_Link( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Revert( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Add_Wants( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Add_Requires( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Edit( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Get_Default( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Set_Default( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_List_Machines( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_List_Jobs( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Cancel( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Show_Environment( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Set_Environment( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Unset_Environment( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Import_Environment( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Daemon_Reload( int argc, wchar_t *argv[] )
{
    g_pool->ReloadPool( );
    return 0;
}


int SystemCtrl_Cmd_Daemon_Reexec( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Is_System_Running( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Default( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Rescue( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Emergency( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Halt( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Poweroff( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Reboot( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Kexec( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Exit( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Switch_Root( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Suspend( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Hibernate( int argc, wchar_t *argv[] )
{
    return -1;
}


int SystemCtrl_Cmd_Hybrid_Sleep( int argc, wchar_t *argv[] )
{
    return -1;
}



typedef int (*cmdfunc)( int argc, wchar_t *argv[] );

std::map< std::wstring , cmdfunc> Command_Map =  {
        { L"list-units", SystemCtrl_Cmd_List_Units},
        { L"list-sockets", SystemCtrl_Cmd_List_Sockets},
        { L"list-timers", SystemCtrl_Cmd_List_Timers},
        { L"start", SystemCtrl_Cmd_Start},
        { L"stop", SystemCtrl_Cmd_Stop},
        { L"reload", SystemCtrl_Cmd_Reload},
        { L"restart", SystemCtrl_Cmd_Restart},
        { L"try-restart", SystemCtrl_Cmd_Try_Restart},
        { L"reload-or-restart", SystemCtrl_Cmd_Reload_Or_Restart},
        { L"try-reload-or-restart", SystemCtrl_Cmd_Try_Reload_Or_Restart},
        { L"isolate", SystemCtrl_Cmd_Isolate},
        { L"kill", SystemCtrl_Cmd_Kill},
        { L"is-active", SystemCtrl_Cmd_Is_Active},
        { L"is-failed", SystemCtrl_Cmd_Is_Failed},
        { L"status", SystemCtrl_Cmd_Status},
        { L"show", SystemCtrl_Cmd_Show},
        { L"cat", SystemCtrl_Cmd_Cat},
        { L"set-property", SystemCtrl_Cmd_Set_Property},
        { L"help", SystemCtrl_Cmd_Help},
        { L"reset-failed", SystemCtrl_Cmd_Reset_Failed},
        { L"list-dependencies", SystemCtrl_Cmd_List_Dependencies},
        { L"list-unit-files", SystemCtrl_Cmd_List_Unit_Files},
        { L"enable", SystemCtrl_Cmd_Enable},
        { L"disable", SystemCtrl_Cmd_Disable},
        { L"reenable", SystemCtrl_Cmd_Reenable},
        { L"preset", SystemCtrl_Cmd_Preset},
        { L"preset-all", SystemCtrl_Cmd_Preset_All},
        { L"is-enabled", SystemCtrl_Cmd_Is_Enabled},
        { L"mask", SystemCtrl_Cmd_Mask},
        { L"unmask", SystemCtrl_Cmd_Unmask},
        { L"link", SystemCtrl_Cmd_Link},
        { L"revert", SystemCtrl_Cmd_Revert},
        { L"add-wants", SystemCtrl_Cmd_Add_Wants},
        { L"add-requires", SystemCtrl_Cmd_Add_Requires},
        { L"edit", SystemCtrl_Cmd_Edit},
        { L"get-default", SystemCtrl_Cmd_Get_Default},
        { L"set-default", SystemCtrl_Cmd_Set_Default},
        { L"list-machines", SystemCtrl_Cmd_List_Machines},
        { L"list-jobs", SystemCtrl_Cmd_List_Jobs},
        { L"cancel", SystemCtrl_Cmd_Cancel},
        { L"show-environment", SystemCtrl_Cmd_Show_Environment},
        { L"set-environment", SystemCtrl_Cmd_Set_Environment},
        { L"unset-environment", SystemCtrl_Cmd_Unset_Environment},
        { L"import-environment", SystemCtrl_Cmd_Import_Environment},
        { L"daemon-reload", SystemCtrl_Cmd_Daemon_Reload},
        { L"daemon-reexec", SystemCtrl_Cmd_Daemon_Reexec},
        { L"is-system-running", SystemCtrl_Cmd_Is_System_Running},
        { L"default", SystemCtrl_Cmd_Default},
        { L"rescue", SystemCtrl_Cmd_Rescue},
        { L"emergency", SystemCtrl_Cmd_Emergency},
        { L"halt", SystemCtrl_Cmd_Halt},
        { L"poweroff", SystemCtrl_Cmd_Poweroff},
        { L"reboot", SystemCtrl_Cmd_Reboot},
        { L"kexec", SystemCtrl_Cmd_Kexec},
        { L"exit", SystemCtrl_Cmd_Exit},
        { L"switch-root", SystemCtrl_Cmd_Switch_Root},
        { L"suspend", SystemCtrl_Cmd_Suspend},
        { L"hibernate", SystemCtrl_Cmd_Hibernate},
        { L"hybrid-sleep", SystemCtrl_Cmd_Hybrid_Sleep}
};


void ParseArgs(int argc, wchar_t *argv[])

{
    int result = 0;

    cmdfunc func = Command_Map[argv[1]];
    if (!func) {
        std::wstringstream msg;
        msg << "Invalid command " << argv[1];
        std::wstring ws = msg.str();
        std::string msgstr = std::string(ws.begin(), ws.end());

        throw std::exception(msgstr.c_str());
    }
    else {
        result = (*func)(argc, argv);

        if (result) {
            std::wstringstream msg;
            msg << "the command " << argv[1] << " is not implemented";
            std::wstring ws = msg.str();
            std::string msgstr = std::string(ws.begin(), ws.end());

            throw std::exception(msgstr.c_str());
        }
    }
}
    


int wmain(int argc, wchar_t *argv[])


{ 
    try {
        g_pool->LoadPool();
        ParseArgs(argc, argv);
    }
    catch(std::exception &e) {
        wcerr << e.what() << endl;
        exit(1);
    }
    exit(0);
}


