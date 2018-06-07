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
#include "wincred.h"
#include "winsvc.h"
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <ios>
#include <LsaLookup.h>
#include <ntsecapi.h>
#include "service_unit.h"


using namespace std;

wstring SystemDUnit::SERVICE_WRAPPER = L"systemd-exec.exe";
wstring SystemDUnitPool::SERVICE_WRAPPER_PATH;

void
SystemDUnit::AddUserServiceLogonPrivilege()

{
    PCREDENTIALW pcred = NULL;
    wstring username;

    {
        //--- RETRIEVE user credentials we need the username in order to get the SID

        BOOL ok = ::CredReadW (L"dcos/app", CRED_TYPE_GENERIC, 0, &pcred);
        if (!ok) {
            wcerr << L"CredRead() failed in AddUserServiceLogonPriveilege - errno " << GetLastError() << std::endl;
        }
        else {
            username = wstring(pcred->UserName); // L"wp128869010\\azureuser"; // 
            wcerr << L"Read username = " << username << std::endl;
        }
        // must free memory allocated by CredRead()!
        ::CredFree (pcred);
    }

    // Get the sid
    SID *psid;
    SID_NAME_USE nameuse;
    DWORD sid_size = 0;
    DWORD domain_size = 0;
    // Get sizes
    wcerr << L"ADD the SERVICELOGON PRIVILEGE" << std::endl;
    (void)LookupAccountNameW(  NULL, // local machine
                           username.c_str(),  // Account name
                           NULL,   // sid ptr
                           &sid_size, // sizeof sid
                           NULL,   // referenced domain
                           &domain_size,
                           &nameuse );
    // ignore the return
    wcerr << L"p2 sid_size = " << sid_size << " domainlen = " << domain_size << std::endl;
    std::vector<char>sid_buff(sid_size);
    psid = (SID*)sid_buff.data();
    std::vector<wchar_t>refdomain(domain_size+1);
    if (!LookupAccountNameW(  NULL, // local machine
                           username.c_str(),  // Account name
                           psid,   // sid ptr
                           &sid_size, // sizeof sid
                           refdomain.data(),   // referenced domain
                           &domain_size,
                           &nameuse )) {
        DWORD err = GetLastError();
        wcerr << L"LookupAccountName() failed in AddUserServiceLogonPrivilege - errno " << GetLastError() << L" sid size " << sid_size << " domain_size << " << domain_size << std::endl;
        return;
    }

    // Get the LSA_HANDLE
    LSA_OBJECT_ATTRIBUTES attrs = {0};
    LSA_HANDLE policy_h;
    DWORD status = LsaOpenPolicy(NULL, &attrs, POLICY_ALL_ACCESS, &policy_h);
    if (status != S_OK) {
        wcerr << L"LsaOpenPolicy() failed in AddUserServiceLogonPrivilege - errno " << status << std::endl;
        return;
    }

    static const std::wstring se_service_logon = L"SeServiceLogonRight";
    LSA_UNICODE_STRING privs[1];
    privs[0].Length = se_service_logon.length()*sizeof(wchar_t);
    privs[0].MaximumLength = se_service_logon.max_size()*sizeof(wchar_t);
    privs[0].Buffer = (wchar_t *)(se_service_logon.c_str());

    status = LsaAddAccountRights( policy_h,
                                  psid,
                                  privs,
                                  1);
    if (status != S_OK) {
        wcerr << L"LsaAddAccountRights() failed in AddUserServiceLogonPrivilege - errno " << status << std::endl;
        return;
    }

    wcerr << L"Service Logon right added" << std::endl;
}



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
        wcerr << L"In StartService(" << this->name << "): OpenService failed " << GetLastError() << std::endl;
        CloseServiceHandle(hsc);
        return false;
    }

    if (!StartServiceW(hsvc, 0, NULL)) {
        DWORD errcode = GetLastError();

        switch(errcode) {
        case ERROR_SERVICE_EXISTS:
            // The service already running is not an error
            wcerr << L"In StartService(" << this->name  << "): StartService failed " << GetLastError() << std::endl;
            CloseServiceHandle(hsvc);
            return false;

        case ERROR_ACCESS_DENIED:
        case ERROR_SERVICE_LOGON_FAILED:

            // The user lacks the necessary privelege. Add it and retry once

            CloseServiceHandle(hsvc); 
            AddUserServiceLogonPrivilege();
            if (!this->m_retry++ ) {
               return  StartService(blocking);
            }
            return false;

        default:
            wcerr << L"In StartService(" << this->name  << "): StartService error =  " << errcode << std::endl;
            return false;
            break;
        }
    }
    
    wcerr << L"In StartService(" << this->name  << "): StartService running " << std::endl;
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
        wcerr << L"In Stop service(" << this->name << "): OpenService failed " << GetLastError() << std::endl;
        CloseServiceHandle(hsc);
        return false;
    }

    SERVICE_STATUS status = { 0 };
    if (ControlService(hsvc, SERVICE_CONTROL_STOP, &status)) {
        wcerr << L"StopService(" << this->name << ") failed " << GetLastError() << std::endl;
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
    {   DWORD last_err = GetLastError();

        if (last_err == ERROR_SERVICE_DOES_NOT_EXIST ||
            last_err == ERROR_SERVICE_DISABLED ) {
            wcerr << L"service " << this->name << " is not enabled " << std::endl;
        }
        else {
            wcerr << L" In IsEnabled error from OpenService " << last_err << std::endl;
        }
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
    // We point at an absolute path based on the location of systemctl.exe.
    // This means that the systemd-exec.exe must be in the same binary
    // directory as the systemctl. 

    HMODULE hModule = GetModuleHandleW(NULL);
    wchar_t path[MAX_PATH];
    
    GetModuleFileNameW(hModule, path, MAX_PATH);
    std::wstring wspath = path;
    int pathend = wspath.find_last_of(L'\\')+1;
    wspath = wspath.substr(0, pathend);

    std::wstring wservice_name         = this->name;
    std::wstring wservice_display_name = this->name;
    std::wstring wdependency_list;

    std::wstringstream wcmdline ;

   // wcmdline << wspath;
    wcmdline << SystemDUnitPool::SERVICE_WRAPPER_PATH.c_str();
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
            wdependency_list.append(wdep);
            wdependency_list.push_back('\0');
        }
    }
    wdependency_list.push_back('\0');

for (wchar_t *pelem = (wchar_t*)wdependency_list.c_str(); pelem < wdependency_list.c_str()+wdependency_list.size(); ) {
wcerr << "dependent: " << pelem << std::endl;
pelem += wcslen(pelem);
if (!*pelem) break;
}

    PCREDENTIALW pcred = NULL;
    wstring username;
    wstring user_password;
    { //--- RETRIEVE user credentials. We need to have credentials specified for the service user otherwise we are
      //    LocalSystem which is a bit too restrictive to be able to set stuff up.

        BOOL ok = ::CredReadW (L"dcos/app", CRED_TYPE_GENERIC, 0, &pcred);
        if (!ok) {
            wcerr << L"CredRead() failed - errno " << GetLastError() << std::endl;
        }
        else {
            user_password = wstring((wchar_t*)pcred->CredentialBlob, pcred->CredentialBlobSize / sizeof(wchar_t));
            username = wstring(pcred->UserName); // L"wp128869010\\azureuser"; // 
            // wcerr << L"Read username = " << username << " password= " << user_password << std::endl;
        }
        // must free memory allocated by CredRead()!
        ::CredFree (pcred);
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
        username.c_str(), //pcred? username.c_str(): NULL,  // LocalSystem account 
        user_password.c_str()); // pcred ? user_password.c_str() : NULL);   // no password 
 
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
        wcerr << L"In unregister: OpenService failed " << GetLastError() << std::endl;
        CloseServiceHandle(hsvc);
        return false;
    }

    if (!DeleteService(hsvc)) {
        wcerr << L"In unregister: DeleteService failed " << GetLastError() << std::endl;
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

