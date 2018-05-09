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
#include "windows.h"
#include "winsvc.h"
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <ios>
#include "service_unit.h"


using namespace std;

wstring SystemDUnit::SERVICE_WRAPPER = L"systemd-exec.exe";

boolean SystemDUnit::StartService(boolean blocking)
{
    SC_HANDLE hsc = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hsc) {
        int last_error = GetLastError();
        wcerr << L"failed to open service manager, err = " << last_error << std::endl;
        return false;
    }

    SC_HANDLE hsvc = OpenServiceW(hsc, this->name.c_str(), SERVICE_ALL_ACCESS);
    if (!hsvc) {
        wcerr << L"OpenService failed " << GetLastError() << std::endl;
        CloseServiceHandle(hsc);
        return false;
    }

    if (!StartServiceW(hsvc, 0, NULL)) {
        wcerr << L"OpenService failed " << GetLastError() << std::endl;
        CloseServiceHandle(hsvc);
        return false;
    }
    
    CloseServiceHandle(hsvc); 
    CloseServiceHandle(hsc);
    
    return true;
}

boolean SystemDUnit::StopService(boolean blocking)
{
    SC_HANDLE hsc = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hsc) {
        int last_error = GetLastError();
        wcerr << "failed to open service manager, err = " << last_error << std::endl;
        return false;
    }

    SC_HANDLE hsvc = OpenServiceW(hsc, this->name.c_str(), SERVICE_ALL_ACCESS);
    if (!hsvc) {
        wcerr << L"OpenService failed " << GetLastError() << std::endl;
        CloseServiceHandle(hsc);
        return false;
    }

    SERVICE_STATUS status = { 0 };
    if (ControlService(hsvc, SERVICE_CONTROL_STOP, &status)) {
        wcerr << L"StopService failed " << GetLastError() << std::endl;
        CloseServiceHandle(hsvc);
        return false;
    }
    
    CloseServiceHandle(hsvc); 
    CloseServiceHandle(hsc);
    
    return true;
}

boolean SystemDUnit::ReloadService(boolean blocking)
{
    return true;
}

boolean SystemDUnit::RestartService(boolean blocking)
{
    StopService(blocking);
    // WaitForStop
    StartService(blocking);
    return true;
}


boolean SystemDUnit::IsEnabled()
{
    SC_HANDLE hsc = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hsc) {
        int last_error = GetLastError();
        wcerr << "failed to open service manager, err = " << last_error << std::endl;
        return false;
    }

    SC_HANDLE hsvc = OpenServiceW(hsc, this->name.c_str(), SERVICE_QUERY_STATUS);
    if (!hsvc)
    {
        wcerr << L"OpenService failed " << GetLastError() << std::endl;
        CloseServiceHandle(hsc);
        return false;
    }
    
    CloseServiceHandle(hsvc); 
    CloseServiceHandle(hsc);
    return true;
}



boolean 
SystemDUnit::RegisterService()

{
    std::wstring wservice_name         = this->name;
    std::wstring wservice_display_name = this->name;
    std::wstring wdependency_list;

    std::wstringstream wcmdline ;

    wcmdline << SERVICE_WRAPPER.c_str();
    wcmdline << L" ";
    wcmdline << L" --service-name ";
    wcmdline << wservice_name.c_str();
    wcmdline << L" ";
    wcmdline << L" --service-unit ";
    wcmdline << SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH;
    wcmdline << "\\";
    wcmdline << wservice_name.c_str();

    for (auto dependent : this->start_dependencies) {
        std::wstring wdep = dependent->name;
        if (wdep.rfind(L".service") != string::npos) {
            wdep.push_back('\0');
            wdependency_list.append(wdep);
        }
    }

    SC_HANDLE hsc = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hsc) {
        int last_error = GetLastError();
        wcerr << L"Could not open service manager win err = " << last_error << std::endl;
        return false;
    }

    SC_HANDLE hsvc = CreateServiceW( 
        hsc,                       // SCM database 
        wservice_name.c_str(),             // name of service 
        wservice_display_name.c_str(),     // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_DEMAND_START,      // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        wcmdline.str().c_str(),    // path to service's binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        wdependency_list.c_str(),  // no dependencies 
        NULL,                      // LocalSystem account 
        NULL);                     // no password 
 
    if (hsvc == NULL) 
    {
        wcerr << L"CreateService failed " << GetLastError() << std::endl; 
        CloseServiceHandle(hsc);
        return false;
    }
    
    CloseServiceHandle(hsvc); 
    CloseServiceHandle(hsc);

    return true;
}


boolean SystemDUnit::UnregisterService()
{
    SC_HANDLE hsc = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hsc) {
        int last_error = GetLastError();
        wcerr << "failed to open service manager, err = " << last_error << std::endl;
        return false;
    }

    SC_HANDLE hsvc = OpenServiceW(hsc, this->name.c_str(), SERVICE_QUERY_STATUS);
    {
        wcerr << L"OpenService failed " << GetLastError() << std::endl;
        CloseServiceHandle(hsvc);
        return false;
    }

    if (!DeleteService(hsvc)) {
        wcerr << L"OpenService failed " << GetLastError() << std::endl;
        CloseServiceHandle(hsvc);
        return false;
    }
    
    CloseServiceHandle(hsvc); 
    CloseServiceHandle(hsc);
    return true;
}

boolean SystemDUnit::CheckForRequisites()
{
    return true;
}

boolean SystemDUnit::WaitForAfters()
{
    return true;
}

