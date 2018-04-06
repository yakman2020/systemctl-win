/*
 * **==============================================================================
 * **
 * ** Copyright (c) Microsoft Corporation. All rights reserved. See file LICENSE
 * ** for license information.
 * **
 * **==============================================================================
 * */
#ifndef __WIN_SYSTEMD_SERVICE_UNIT_H__
#define __WIN_SYSTEMD_SERVICE_UNIT_H__
#include <iostream>
#include <fstream>
#include "windows.h"
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>

using namespace std;

#define MAX_BUFFER_SIZE 4096
extern wchar_t BUFFER[];
class SystemDUnit;
extern class SystemDUnitPool *g_pool;

class SystemDUnitPool {
public:
    SystemDUnitPool() {
               wstring system_drive;
               DWORD rslt = GetEnvironmentVariableW(
                    L"SystemDrive", BUFFER, MAX_BUFFER_SIZE);
             
               if (rslt == 0) {
                   system_drive = L"C:";
               }
               else {
                   system_drive = BUFFER;
               }
               
               UNIT_DIRECTORY_PATH = system_drive + L"\\etc\\SystemD\\system";
               ACTIVE_UNIT_DIRECTORY_PATH = system_drive + L"\\etc\\SystemD\\active";
               UNIT_WORKING_DIRECTORY_PATH = system_drive + L"\\etc\\SystemD/\\un";
          };

    ~SystemDUnitPool() { };
 
    std::map<std::wstring, class SystemDUnit*>&GetPool() { return pool; };
    static class SystemDUnit *FindUnit(std::wstring name);
    static class SystemDUnit *ReadServiceUnit(std::wstring name, std::wstring service_unit_path);

    // Disables, recopies, reloads, then start/enables everybody in the systemd/lib directory
    void ReloadPool();

    // deserialises and loads the pool of objects. Throws an exception on failure

    void LoadPool();
    void SavePool() { };

   static wstring UNIT_DIRECTORY_PATH;        // Quasi constant inited in contructor

private:
   friend class SystemDUnit;
   static boolean Apply(wstring dir_path, boolean (*action)(wstring dir_path, void *context ), void *context);
   static wstring ACTIVE_UNIT_DIRECTORY_PATH; // Quasi constant
   static wstring UNIT_WORKING_DIRECTORY_PATH; // Quasi constant
   static std::map<std::wstring, class SystemDUnit *> pool;
};


/* Exec action prefix characters:
 *
 * Prefix   Effect
 * "@"	If the executable path is prefixed with "@", the second specified token will be passed 
 *      as "argv[0]" to the executed process (instead of the actual filename), followed by the 
 *      further arguments specified.
 * "-"	If the executable path is prefixed with "-", an exit code of the command normally considered 
 *      a failure (i.e. non-zero exit status or abnormal exit due to signal) is ignored and considered success.
 * "+"	If the executable path is prefixed with "+" then the process is executed with full privileges.
        In this mode privilege restrictions configured with User=, Group=, CapabilityBoundingSet= or the 
 *      various file system namespacing options (such as PrivateDevices=, PrivateTmp=) are not applied to
 *      the invoked command line (but still affect any other ExecStart=, ExecStop=).
 * "!"	Similar to the "+" character discussed above this permits invoking command lines with elevated privileges.
 *      However, unlike "+" the "!" character exclusively alters the effect of User=, Group= and SupplementaryGroups=,
 *      i.e. only the stanzas the affect user and group credentials. Note that this setting may be combined
 *      with DynamicUser=, in which case a dynamic user/group pair is allocated before the command is invoked,
 *      but credential changing is left to the executed process itself.
 *  
 * "!!"	This prefix is very similar to "!", however it only has an effect on systems lacking support for ambient
 *      process capabilities, i.e. without support for AmbientCapabilities=. It's intended to be used for unit
 *      files that take benefit of ambient capabilities to run processes with minimal privileges wherever possible
 *      while remaining compatible with systems that lack ambient capabilities support. Note that when "!!" is used,
 *      and a system lacking ambient capability support is detected any configured SystemCallFilter= and
 *      CapabilityBoundingSet= stanzas are implicitly modified, in order to permit spawned processes to drop
 *      credentials and capabilities themselves, even if this is configured to not be allowed. Moreover, if 
 *      this prefix is used and a system lacking ambient capability support is detected AmbientCapabilities=
 *      will be skipped and not be applied. On systems supporting ambient capabilities, "!!" has no effect
 *      and is redundant.
 *  
 *  "@", "-", and one of "+"/"!"/"!!" may be used together and they can appear in any order. 
 *  However, only one of "+", "!", "!!" may be used at a time. Note that these prefixes are also 
 *  supported for the other command line settings, i.e. ExecStartPre=, ExecStartPost=, ExecReload=,
 *  ExecStop= and ExecStopPost=. 
 */

class SystemDUnit {
public:

    // Constants used to create the output and error bitmasks
    static const unsigned OUTPUT_TO_JOURNAL = 0x01;
    static const unsigned OUTPUT_TO_CONSOLE = 0x02;
    static const unsigned OUTPUT_TO_FILE    = 0x04;

    enum ServiceType {
       SERVICE_TYPE_UNDEFINED,
       SERVICE_TYPE_SIMPLE,
       SERVICE_TYPE_FORKING,
       SERVICE_TYPE_ONESHOT, 
       SERVICE_TYPE_DBUS,
       SERVICE_TYPE_NOTIFY,
       SERVICE_TYPE_IDLE
    };

   // StandardOut and StandardError are specified as type and maybe name
   // most of these will have significant differences in their semantics from unix/linux.
   enum OUTPUT_TYPE {
       OUTPUT_TYPE_INVALID,
       OUTPUT_TYPE_INHERIT,
       OUTPUT_TYPE_NULL,
       OUTPUT_TYPE_TTY,
       OUTPUT_TYPE_JOURNAL,
       OUTPUT_TYPE_SYSLOG,
       OUTPUT_TYPE_KMSG,
       OUTPUT_TYPE_JOURNAL_PLUS_CONSOLE,
       OUTPUT_TYPE_SYSLOG_PLUS_CONSOLE,
       OUTPUT_TYPE_KMSG_PLUS_CONSOLE,
       OUTPUT_TYPE_FILE,   // requires a path
       OUTPUT_TYPE_SOCKET,
       OUTPUT_TYPE_FD      // requires a name
    };

    SystemDUnit(wchar_t *name) {
            this->name = name;
            this->is_enabled        = false;
            this->remain_after_exit = false;
            g_pool->GetPool().insert(std::make_pair(name, this));
        };
    ~SystemDUnit() {  };

  
    boolean Enable(boolean block);
    boolean Disable(boolean block);
    boolean Mask(boolean block);
    boolean Unmask(boolean block);

    boolean IsEnabled() ;
    wstring  &Name()  { return name; };

    void AddBefore(wstring servicename) {
                     before.push_back(servicename);
                 };

    vector<wstring>&GetBefore() { return before; };

    void AddAfter(wstring servicename) {
                     after.push_back(servicename);
                 };
    vector<wstring>&GetAfter() { return after; };

    void AddRequire(wstring servicename) { 
                     requires.push_back(servicename);
                 };

    vector<wstring>&GetRequires() { return requires; };

    void AddRequiredBy(wstring servicename) { 
                     required_by.push_back(servicename);
                 };

    vector<wstring>&GetRequiredBy() { return required_by; };

    // Things that get started (first) when we start
    // based on required_by and requires
    void AddStartDependency(SystemDUnit *dependency) {
                     start_dependencies.push_back(dependency);
                 };    
    vector<class SystemDUnit *> GetStartDependencies()  {
                     return start_dependencies;
                 }; 

    // Things that get must be started before we start
    // based on Before and After
    void AddWaitDependency(SystemDUnit *dependency) {
                     wait_dependencies.push_back(dependency);
                 };

    vector<class SystemDUnit *> GetWaitDependencies()  {
                     return wait_dependencies;
                 };

    static class SystemDUnit *ParseSystemDServiceUnit(wstring servicename, wifstream &fs);
 
    boolean StartService(boolean blocking);
    boolean StopService(boolean blocking);
    boolean ReloadService(boolean blocking);
    boolean RestartService(boolean blocking);

    boolean RegisterService();
    boolean UnregisterService();
    boolean CheckForRequisites();
    boolean WaitForAfters();

private:
    wstring ParseUnitSection( wifstream &fs);
    wstring ParseServiceSection( wifstream &fs);
    wstring ParseInstallSection( wifstream &fs);

    wstring name;
    wstring description;
    boolean is_enabled;
    enum OUTPUT_TYPE output_type;
    enum OUTPUT_TYPE error_type;
    wstring   output_file_path;
    wstring   error_file_path;
    wstring   input_data;         // Receives data from StandardInputText or StandardInputData. Held as binary data
    boolean remain_after_exit;
    boolean guess_main_pid;      // only interesting if type == forking
    wstring  pid_file;            // aboslute path of the pid file of the service. recommended for type = forking
    wstring  bus_name;            // d-bus bus name (in windows this is just a service id) . Required for type = dbus
    enum ServiceType service_type;

    double restart_sec;             // time to sleep before restarting a service. Default 100ms
    double timeout_start_sec;       // time to wait for startup. -1 is "infinity", Defaults to DefautTimeoutStopSec
    double timeout_stop_sec;
    double watchdog_sec;
    double max_runtime_sec;

    vector<wstring> before;   // Kept in order of definition
    vector<wstring> after;    // Kept in order of definition
    vector<wstring> wants;    // Kept in order of definition
    vector<wstring> wanted_by; // Kept in order of definition

    vector<wstring> requires;    // Kept in order of definition
    vector<wstring> requisite; // Kept in order of definition
    vector<wstring> required_by; // Kept in order of definition

    vector<wstring> exec_start_pre;  // Kept in order of definition
    vector<wstring> exec_start;      // Kept in order of definintion
    vector<wstring> exec_start_post; // Kept in order of definition
    vector<wstring> exec_reload;     // Kept in order of definintion
    vector<wstring> exec_stop;       // Kept in order of definintion
    vector<wstring> exec_stop_post;  // Kept in order of definition

    vector<wstring> environment_file;  // Kept in order of definition
    vector<wstring> environment_vars;  // Kept in order of definition

    vector<class SystemDUnit *> start_dependencies; // Requires 
    vector<class SystemDUnit *> wait_dependencies;  // After

    static wstring SERVICE_WRAPPER;
};


#endif // __WIN_SYSTEMD_SERVICE_UNIT_H__
