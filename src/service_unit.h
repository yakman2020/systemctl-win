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
    SystemDUnitPool();
    ~SystemDUnitPool() { };
 
    std::map<std::wstring, class SystemDUnit*>&GetPool() { return pool; };
    static class SystemDUnit *FindUnit(std::wstring name);
    static wstring FindServiceFilePath(wstring dir_path, wstring service_name); 
    static class SystemDUnit *ReadServiceUnit(std::wstring name, std::wstring service_unit_path);

    // Actual unit files only occur in the top directory. Various before, after, wants and requires directories are 
    // hard links to the actual unit file in ACTIVE_UNIT_DIRECTORY_PATH\\servicename
    static boolean LinkWantedUnit(wstring wanted_unit_path, wstring servicename);
    static boolean CopyUnitFileToActive(wstring servicename);

    // Disables, recopies, reloads, then start/enables everybody in the systemd/lib directory
    void ReloadPool();

    // deserialises and loads the pool of objects. Throws an exception on failure

    void LoadPool();
    void SavePool() { };

   static wstring UNIT_DIRECTORY_PATH;        // Quasi constant inited in contructor
   static wstring SERVICE_WRAPPER_PATH; // Single instance for everybody

private:
   friend class SystemDUnit;
   static boolean DirExists(wstring dir_path);
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

    SystemDUnit(const wchar_t *name, const wchar_t *file_path = NULL) {
            this->name = name;
            if (file_path) {
                wstring dos_path = file_path ;
                std::replace_if(dos_path.begin(), dos_path.end(),
                        [](wchar_t c) -> bool
                            {
                                return c == '/';
                            }, '\\');
    
                // The path should be relative. We will strip off the 
                // base directory paths if known
                if (wcsncmp(dos_path.c_str(), SystemDUnitPool::UNIT_DIRECTORY_PATH.c_str(), 
                           SystemDUnitPool::UNIT_DIRECTORY_PATH.length()) == 0) {
                    this->unit_file_path = dos_path.c_str() + SystemDUnitPool::UNIT_DIRECTORY_PATH.length();
                }
                else if (wcsncmp(dos_path.c_str(), SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH.c_str(), 
                           SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH.length()) == 0) {
                    this->unit_file_path = dos_path.c_str() + SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH.length();
                }
                else {
                    this->unit_file_path = dos_path;
                }
            }
            else {
                this->unit_file_path = name;
            }
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
    wstring  &FilePath()  { return unit_file_path; };
    wstring  &FilePath(wstring &file_path)  {
            if (!file_path.empty()) {
                wstring dos_path = file_path ;
                std::replace_if(dos_path.begin(), dos_path.end(),
                        [](wchar_t c) -> bool
                            {
                                return c == '/';
                            }, '\\');
    
                // The path should be relative. We will strip off the 
                // base directory paths if known
                if (wcsncmp(dos_path.c_str(), SystemDUnitPool::UNIT_DIRECTORY_PATH.c_str(), 
                           SystemDUnitPool::UNIT_DIRECTORY_PATH.length()) == 0) {
                    this->unit_file_path = dos_path.c_str() + SystemDUnitPool::UNIT_DIRECTORY_PATH.length();
                }
                else if (wcsncmp(dos_path.c_str(), SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH.c_str(), 
                           SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH.length()) == 0) {
                    this->unit_file_path = dos_path.c_str() + SystemDUnitPool::ACTIVE_UNIT_DIRECTORY_PATH.length();
                }
                else {
                    this->unit_file_path = dos_path;
                }
            }
            else {
                this->unit_file_path = name;
            }
            return unit_file_path;
        };

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

    void AddWanted(wstring servicename) { 
                     wants.push_back(servicename);
                 };

    vector<wstring>&GetWants() { return wants; };

    void AddWantedBy(wstring servicename) { 
                     wanted_by.push_back(servicename);
                 };

    vector<wstring>&GetWantedBy() { return wanted_by; };

    // Things that get started (first) when we start
    // based on wanted_by, wants, required_by and requires
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

    static class SystemDUnit *ParseSystemDServiceUnit(wstring servicename, wstring unit_path, wifstream &fs);
 
    boolean StartService(boolean blocking);
    boolean StopService(boolean blocking);
    boolean ReloadService(boolean blocking);
    boolean RestartService(boolean blocking);

    boolean RegisterService();
    boolean UnregisterService();
    boolean CheckForRequisites();
    boolean WaitForAfters();
    boolean SystemDUnit::Kill(int action, std::wstring killtarget, boolean block);


private:
    wstring ParseUnitSection( wifstream &fs);
    wstring ParseServiceSection( wifstream &fs);
    wstring ParseInstallSection( wifstream &fs);

    wstring name;
    wstring description;
    wstring unit_file_path;
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

    enum RestartAction restart_action; // Restart= attribute parsed.
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

    static wstring SERVICE_WRAPPER; // Single instance

    typedef boolean (SystemDUnit::*systemd_service_attr_func)( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    static std::map< std::wstring , systemd_service_attr_func> SystemD_Service_Attribute_Map;

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

    boolean SystemDUnit::attr_service_type( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_remain_after_exit( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_guess_main_pid( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_pid_file( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_bus_name( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_exec_start_pre( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_exec_start( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_exec_start_post( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_exec_stop( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_exec_stop_post( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_exec_reload( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_restart_sec( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_timeout_start_sec( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_timeout_stop_sec( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_timeout_sec( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_runtime_max_sec( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_watchdog_sec( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_restart( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_success_exit_status( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_restart_prevent_exit_status( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_restart_force_exit_status( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_permissions_start_only( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_root_directory_start_only( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_non_blocking( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_notify_access( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_sockets( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_file_descriptor_store_max( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_usb_function_descriptors( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_usb_function_strings( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_environment( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_environment_file( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_standard_output( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_standard_error( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );
    boolean SystemDUnit::attr_not_implemented( wstring attr_name, wstring attr_value, unsigned long &attr_bitmask );

};


#endif // __WIN_SYSTEMD_SERVICE_UNIT_H__
