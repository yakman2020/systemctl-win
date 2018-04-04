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
#include <vector>
#include <ios>
#include <limits>
#include <string>
#include <cstddef>
#include "service_unit.h"

using namespace std;


char BUFFER[MAX_BUFFER_SIZE] = { '\0' };

extern class SystemDUnitPool *g_pool;


class SystemDUnit *SystemDUnit::ParseSystemDServiceUnit(string servicename, fstream &fs)
{ 
    cerr << "ParseSystemDServiceUnit\n";
   std::string line;
   class SystemDUnit *punit = new class SystemDUnit((char*)servicename.c_str());
 
    while (true) {
        if (fs.eof()) {
            break;
        }
        (void) fs.getline(BUFFER, MAX_BUFFER_SIZE);
        line = BUFFER;
        line.erase(
            std::remove_if(line.begin(), line.end(), 
                    [](char c) -> bool
                    { 
                        return std::isspace(c); 
                    }), 
                line.end());
        
        if (line.compare("[Unit]") != string::npos) {
             // Then we need to parse the unit section
             cerr << "parse unit section\n";
             punit->ParseUnitSection(fs);
        }
        else if (line.compare("[Service]") != string::npos) {
             // Then we need to parse the service section
             cerr << "parse service section\n";
             punit->ParseServiceSection(fs);
        }
        else if (line.compare("[Install]") != string::npos) {
             // Then we need to parse the install section
             cerr << "parse install section\n";
             punit->ParseInstallSection(fs);
        }
    }
   
    return punit;
}
 

// When it returns this will be located at EOF or the next section ([Service] for example).

static inline void split_elems(fstream &fs, vector<string> &attrs, vector<string> &values)

{
    int prev_pos = 0;
    string attrname;
    string attrval;

    while (true) {
        try { 
            if (fs.eof()) {
                break;
            }
            prev_pos = fs.tellg();
            (void) fs.getline(BUFFER, MAX_BUFFER_SIZE);
        }
         catch (const std::exception &e) {
            cerr << e.what() << endl;
            break;
        }

        string line = BUFFER;
        int first_non_space = line.find_first_not_of(" \t\r\n");
        int split_pt  = line.find_first_of("=");
        if (split_pt == std::string::npos) {
            if (first_non_space == std::string::npos) {
                continue; // a blank line or eof
             }
             // If this line is part of the next section, we put it back
             if (line[first_non_space] == '[') {
                 fs.seekg(prev_pos, std::ios_base::beg);
             }
             break;
        }

        attrname = line.substr(first_non_space, split_pt);

        int last_non_space = line.find_last_not_of(" \t\r\n");
        if (last_non_space == std::string::npos) {
            last_non_space = line.length();
        }
        attrval  = line.substr(split_pt+1, last_non_space );
        attrs.push_back(string(attrname));
        values.push_back(string(attrval));
        cerr << "attr = " << attrname.c_str() << endl;
    }
}


boolean SystemDUnit::ParseUnitSection( fstream &fs)

{ 
    vector<string> attrs;
    vector<string> values;

    split_elems(fs, attrs, values);
   
    for (auto i = 0; i < attrs.size(); i++) {
        if (attrs[i].compare("Description") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("Documentation") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("Requires") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("Requisite") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("Wants") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("BindsTo") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("PartOf") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("Conflicts") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("Before") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("After") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("OnFailure") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("PropagatesReloadTo") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ReloadPropagatedFrom") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("JoinsNamespaceOf") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("RequiresMountsFor") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("OnFailureJobMode") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("IgnoreOnIsolate") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("StopWhenUnneeded") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("RefuseManualStart") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("RefuseManualStop") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AllowIsolate") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("DefaultDependencies") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("CollectMode") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("JobTimeoutSec") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("JobRunningTimeoutSec") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("JobTimeoutAction") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("JobTimeoutRebootArgument") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("StartLimitIntervalSec") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("StartLimitBurst") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("StartLimitAction") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("FailureAction") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("SuccessAction") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("RebootArgument") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionArchitecture") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionVirtualization") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionHost") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionKernelCommandLine") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionKernelVersion") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionSecurity") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionCapability") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionACPower") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionNeedsUpdate") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionFirstBoot") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionPathExists") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionPathExistsGlob") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionPathIsDirectory") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionPathIsSymbolicLink") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionPathIsMountPoint") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionPathIsReadWrite") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionDirectoryNotEmpty") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionFileNotEmpty") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionFileIsExecutable") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionUser") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionGroup") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConditionControlGroupController") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertArchitecture") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertVirtualization") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertHost") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertKernelCommandLine") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertKernelVersion") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertSecurity") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertCapability") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertACPower") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertNeedsUpdate") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertFirstBoot") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertPathExists") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertPathExistsGlob") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertPathIsDirectory") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertPathIsSymbolicLink") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertPathIsMountPoint") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertPathIsReadWrite") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertDirectoryNotEmpty") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertFileNotEmpty") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertFileIsExecutable") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertUser") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertGroup") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AssertControlGroupController") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else {
            cerr << "attribute not recognised: " << attrs[i].c_str() << endl;
        }
    }
    
    return true;
}

boolean SystemDUnit::ParseServiceSection( fstream &fs)

{   
    static const unsigned long  ATTRIBUTE_BIT_TYPE                       =  0x00000001;
    static const unsigned long  ATTRIBUTE_BIT_REMAIN_AFTER_EXIT          =  0x00000002;
    static const unsigned long  ATTRIBUTE_BIT_GUESS_MAIN_PID             =  0x00000004;
    static const unsigned long  ATTRIBUTE_BIT_PID_FILE                   =  0x00000008;
    static const unsigned long  ATTRIBUTE_BIT_BUS_NAME                   =  0x00000010;
    static const unsigned long  ATTRIBUTE_BIT_EXEC_START_PRE             =  0x00000020;
    static const unsigned long  ATTRIBUTE_BIT_EXEC_START                 =  0x00000040;
    static const unsigned long  ATTRIBUTE_BIT_EXEC_START_POST            =  0x00000080;
    static const unsigned long  ATTRIBUTE_BIT_EXEC_RELOAD                =  0x00000100;
    static const unsigned long  ATTRIBUTE_BIT_EXEC_STOP                  =  0x00000200;
    static const unsigned long  ATTRIBUTE_BIT_EXEC_STOP_POST             =  0x00000400;
    static const unsigned long  ATTRIBUTE_BIT_RESTART_SEC                =  0x00000800;
    static const unsigned long  ATTRIBUTE_BIT_TIMEOUT_START_SEC          =  0x00001000;
    static const unsigned long  ATTRIBUTE_BIT_TIMEOUT_STOP_SEC           =  0x00002000;
    static const unsigned long  ATTRIBUTE_BIT_RUNTIME_MAX_SEC            =  0x00008000;
    static const unsigned long  ATTRIBUTE_BIT_WATCHDOG_SEC               =  0x00010000;
    static const unsigned long  ATTRIBUTE_BIT_RESTART                    =  0x00020000;
    static const unsigned long  ATTRIBUTE_BIT_SUCCESS_EXIT_STATUS        =  0x00040000;
    static const unsigned long  ATTRIBUTE_BIT_RESTART_PREVENT_EXIT_STATUS =  0x00080000;
    static const unsigned long  ATTRIBUTE_BIT_RESTART_FORCE_EXIT_STATUS  =  0x00100000;
    static const unsigned long  ATTRIBUTE_BIT_PERMISSIONS_START_ONLY     =  0x00200000;
    static const unsigned long  ATTRIBUTE_BIT_ROOT_DIRECTORY_START_ONLY  =  0x00400000;
    static const unsigned long  ATTRIBUTE_BIT_NON_BLOCKING               =  0x00800000;
    static const unsigned long  ATTRIBUTE_BIT_NOTIFY_ACCESS              =  0x01000000;
    static const unsigned long  ATTRIBUTE_BIT_SOCKETS                    =  0x02000000;
    static const unsigned long  ATTRIBUTE_BIT_FILE_DESCRIPTOR_STORE_MAX  =  0x04000000;
    static const unsigned long  ATTRIBUTE_BIT_USB_FUNCTION_DESCRIPTORS   =  0x08000000;
    static const unsigned long  ATTRIBUTE_BIT_USB_FUNCTION_STRINGS       =  0x10000000;

    unsigned long attr_bitmask = 0;
    vector<string> attrs;
    vector<string> values;

    enum ServiceType attr_type = SERVICE_TYPE_UNDEFINED;
    boolean attr_remainafterexit = false;
    boolean attr_guessmainpid    = true;
    string  attr_pidfile  = "";
    string  attr_busname  = "";
    vector<string> attr_execstart;

    split_elems(fs, attrs, values);

    for (auto i = 0; i < attrs.size(); i++) {

        if (attrs[i].compare("Type") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_TYPE;
            
            if (values[i].compare("simple") == 0) {
                attr_type = SERVICE_TYPE_SIMPLE;
            }
            else if (values[i].compare("forking") == 0) {
                attr_type = SERVICE_TYPE_FORKING;
            }
            else if (values[i].compare("oneshot") == 0) {
                attr_type = SERVICE_TYPE_ONESHOT;
            }
            else if (values[i].compare("dbus") == 0) {
                attr_type = SERVICE_TYPE_DBUS;
            }
            else if (values[i].compare("notify") == 0) {
                attr_type = SERVICE_TYPE_NOTIFY;
            }
            else if (values[i].compare("idle") == 0) {
                attr_type = SERVICE_TYPE_IDLE;
            }
            else {
                attr_type = SERVICE_TYPE_UNDEFINED;
                cerr << "service type " << values[i].c_str() << "is unknown" << endl;
            }
        }
        else if (attrs[i].compare("RemainAfterExit") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_REMAIN_AFTER_EXIT;
            if (values[i].compare("yes") == 0) {
                attr_remainafterexit = true;
            }
            else if (values[i].compare("no") == 0) {
                attr_remainafterexit = false;
            }
            else {
                attr_remainafterexit = false;
            }
            this->remain_after_exit = attr_remainafterexit;
        }
        else if (attrs[i].compare("GuessMainPID") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_GUESS_MAIN_PID;
            if (values[i].compare("yes") == 0) {
                attr_guessmainpid = true;
            }
            else if (values[i].compare("no") == 0) {
                attr_guessmainpid = false;
            }
            else {
                attr_guessmainpid = false;
            }
            this->guess_main_pid = attr_guessmainpid;
        }
        else if (attrs[i].compare("PIDFile") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_PID_FILE;
            this->pid_file = values[i];
        }
        else if (attrs[i].compare("BusName") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_BUS_NAME;
            this->bus_name = values[i];
        }
        else if (attrs[i].compare("ExecStartPre") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_EXEC_START_PRE;
            this->exec_start_pre.push_back(values[i]);
        }
        else if (attrs[i].compare("ExecStart") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_EXEC_START;
            this->exec_start.push_back(values[i]);
        }
        else if (attrs[i].compare("ExecStartPost") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_EXEC_START_POST;
            this->exec_start_post.push_back(values[i]);
        }
        else if (attrs[i].compare("ExecReload") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_EXEC_RELOAD;
            this->exec_reload.push_back(values[i]);
        }
        else if (attrs[i].compare("ExecStop") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_EXEC_STOP;
            this->exec_stop.push_back(values[i]);
        }
        else if (attrs[i].compare("ExecStopPost") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_EXEC_STOP_POST;
            this->exec_stop_post.push_back(values[i]);
        }
        else if (attrs[i].compare("RestartSec") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_RESTART_SEC;
            if (values[i].compare("infinity") == 0 ) {
                this->restart_sec = DBL_MAX;
            }
            else {
                try {
                   this->restart_sec = std::stod(values[i]);
                }
                catch (const std::exception &e) {
                    cerr << "restart_sec invalid value : " << values[i].c_str() << endl;
                }
            }
        }
        else if (attrs[i].compare("TimeoutStartSec") == 0) {
            attr_bitmask |= ATTRIBUTE_BIT_TIMEOUT_START_SEC;
            if (values[i].compare("infinity") == 0 ) {
                this->timeout_start_sec = DBL_MAX;
            }
            else {
                try {
                   this->timeout_start_sec = stod(values[i]);
                }
                catch (const std::exception &e) {
                    cerr << "timeout_start_sec invalid value : " << values[i].c_str() << endl;
                }
            }
        }
        else if (attrs[i].compare("TimeoutStopSec") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_TIMEOUT_STOP_SEC;
            if (values[i].compare("infinity") == 0 ) {
                this->timeout_stop_sec = DBL_MAX;
            }
            else {
                try {
                    this->timeout_stop_sec = stod(values[i]);
                }
                catch (const std::exception &e) {
                    // Try to convert from time span like "5min 20s"
                    cerr << "timeout_stop_sec invalid value : " << values[i].c_str() << endl;
                }
            }
        }
        else if (attrs[i].compare("TimeoutSec") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= (ATTRIBUTE_BIT_TIMEOUT_START_SEC | ATTRIBUTE_BIT_TIMEOUT_STOP_SEC);
            if (values[i].compare("infinity") == 0) {
                this->timeout_start_sec = DBL_MAX;
                this->timeout_stop_sec = DBL_MAX;
            }
            else {
                try {
                   auto val = stod(values[i]);
                   this->timeout_stop_sec = val;
                   this->timeout_stop_sec = val;
                }
                catch (const std::exception &e) {
                    // Try to convert from time span like "5min 20s"
                    cerr << "timeout_sec invalid value : " << values[i].c_str() << endl;
                }
            }
        }
        else if (attrs[i].compare("RuntimeMaxSec") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_RUNTIME_MAX_SEC;
            if (values[i].compare("infinity") == 0) {
                this->max_runtime_sec = DBL_MAX;
            }
            else {
                try {
                    this->max_runtime_sec = stod(values[i]);
                }
                catch (const std::exception &e) {
                    // Try to convert from time span like "5min 20s"
                    cerr << "RuntimeMaxSec invalid value : " << values[i].c_str() << endl;
                }
            }
        }
        else if (attrs[i].compare("WatchdogSec") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_WATCHDOG_SEC;
            if (values[i].compare("infinity") == 0 ) {
                this->watchdog_sec = DBL_MAX;
            }
            else {
                try {
                    this->watchdog_sec = stod(values[i]);
                }
                catch (const std::exception &e) {
                    // Try to convert from time span like "5min 20s"
                    cerr << "watchdog_sec invalid value : " << values[i].c_str() << endl;
                }
            }
        }
        else if (attrs[i].compare("Restart") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_RESTART;
        }
        else if (attrs[i].compare("SuccessExitStatus") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_SUCCESS_EXIT_STATUS;
        }
        else if (attrs[i].compare("RestartPreventExitStatus") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_RESTART_PREVENT_EXIT_STATUS;
        }
        else if (attrs[i].compare("RestartForceExitStatus") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_RESTART_FORCE_EXIT_STATUS;
        }
        else if (attrs[i].compare("PermissionsStartOnly") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_PERMISSIONS_START_ONLY;
        }
        else if (attrs[i].compare("RootDirectoryStartOnly") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_ROOT_DIRECTORY_START_ONLY;
        }
        else if (attrs[i].compare("NonBlocking") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_NON_BLOCKING;
        }
        else if (attrs[i].compare("NotifyAccess") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_NOTIFY_ACCESS;
        }
        else if (attrs[i].compare("Sockets") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_SOCKETS;
        }
        else if (attrs[i].compare("FileDescriptorStoreMax") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_FILE_DESCRIPTOR_STORE_MAX;
        }
        else if (attrs[i].compare("USBFunctionDescriptors") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_USB_FUNCTION_DESCRIPTORS;
        }
        else if (attrs[i].compare("USBFunctionStrings") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
            attr_bitmask |= ATTRIBUTE_BIT_USB_FUNCTION_STRINGS;
        }
        else if (attrs[i].compare("WorkingDirectory") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("RootDirectory") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("RootImage") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("MountAPIVFS") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("BindPaths") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("BindReadOnlyPaths") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("User") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("Group") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("DynamicUser") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("SupplementaryGroups") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("PAMName") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("CapabilityBoundingSet") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AmbientCapabilities") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("NoNewPrivileges") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("SecureBits") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("SELinuxContext") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("AppArmorProfile") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("SmackProcessLabel") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("UMask") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("KeyringMode") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("OOMScoreAdjust") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("TimerSlackNSec") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("Personality") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("IgnoreSIGPIPE") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("Nice") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("CPUSchedulingPolicy") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("CPUSchedulingPriority") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("CPUSchedulingResetOnFork") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("CPUAffinity") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("IOSchedulingClass") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("IOSchedulingPriority") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ProtectSystem") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ProtectHome") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("RuntimeDirectory") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("StateDirectory") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("CacheDirectory") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("LogsDirectory") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConfigurationDirectory") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("RuntimeDirectoryMode") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("StateDirectoryMode") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("CacheDirectoryMode") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("LogsDirectoryMode") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ConfigurationDirectoryMode") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("RuntimeDirectoryPreserve") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ReadWritePaths") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ReadOnlyPaths") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("InaccessiblePaths") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("TemporaryFileSystem") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("PrivateTmp") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("PrivateDevices") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("PrivateNetwork") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("PrivateUsers") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ProtectKernelTunables") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ProtectKernelModules") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("ProtectControlGroups") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("RestrictAddressFamilies") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("RestrictNamespaces") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("LockPersonality") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("MemoryDenyWriteExecute") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("RestrictRealtime") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("RemoveIPC") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("MountFlags") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("SystemCallFilter") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("SystemCallErrorNumber") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("SystemCallArchitectures") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("Environment") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("EnvironmentFile") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("PassEnvironment") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("UnsetEnvironment") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("StandardInput") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("StandardOutput") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("StandardError") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("StandardInputText") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("StandardInputData") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("LogLevelMax") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("LogExtraFields") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("SyslogIdentifier") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("SyslogFacility") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("SyslogLevel") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("SyslogLevelPrefix") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("TTYPath") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("TTYReset") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("TTYVHangup") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("TTYVTDisallocate") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("UtmpIdentifier") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("UtmpMode") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else {
            cerr << "attribute not recognised: " << attrs[i].c_str() << endl;
        }
    }

    // if type is not set, then we have default value combinatiosn that
    // produce a type value. We behvae after as if they had been specified that way

    if (! (attr_bitmask & ATTRIBUTE_BIT_TYPE)) {
        
        if ((attr_bitmask & ATTRIBUTE_BIT_EXEC_START) ) {
            if (! (attr_bitmask & ATTRIBUTE_BIT_BUS_NAME) ) {
                attr_type = SERVICE_TYPE_SIMPLE;
                attr_bitmask |= ATTRIBUTE_BIT_TYPE;
            }
            else {
                attr_type = SERVICE_TYPE_DBUS;
                attr_bitmask |= ATTRIBUTE_BIT_TYPE;
            }
        }
        else {
            attr_type = SERVICE_TYPE_ONESHOT;
            attr_bitmask |= ATTRIBUTE_BIT_TYPE;
        }
    }

    // Validate the combinations of definitions


    // Set the values if value
    this->service_type = attr_type;

    return true;
}

boolean SystemDUnit::ParseInstallSection( fstream &fs)

{
    vector<string> attrs;
    vector<string> values;

    split_elems(fs, attrs, values);
    for (auto i = 0; i < attrs.size(); i++) {
        if (attrs[i].compare("Alias") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("WantedBy") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("RequiredBy") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("Also") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else if (attrs[i].compare("DefaultInstance") == 0) {
            cout << "2do: attrs = " << attrs[i].c_str() << " value = " << values[i].c_str() << endl;
        }
        else {
            cerr << "attribute not recognised: " << attrs[i].c_str() << endl;
        }
    }
    return true;
}
