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
#include "service_unit.h"

using namespace std;

boolean SystemDUnit::StartServiceOneShot(boolean blocking)

{
    // Start a one shot service. 
    // First create an ordered dependency graph shuffling the various needs and wants lists. Then 
    // start accroding to type

    // For each dependency, start. 

    // Start this service: 
    // We don't bother with the service manager for a oneshot, since we are just executing the commands (once)
    // In this case, the advantage is the dependencies

    // Execute Start Pre:
    // wait for it?
    // Execute Start
    // wait for it?
    // Execute Start Post
    // wait for it?

}


boolean SystemDUnit::StartServiceSimple(boolean blocking)

{
    /* --- 2DO ---- */
    // Start a one shot service. 
    // First create an ordered dependency graph shuffling the various needs and wants lists. Then 
    // start accroding to type

    // For each dependency, start. 

    // Start this service: 
    // We don't bother with the service manager for a oneshot, since we are just executing the commands (once)
    // In this case, the advantage is the dependencies

    // Execute Start Pre:
    // wait for it?
    // Execute Start
    // wait for it?
    // Execute Start Post
    // wait for it?

}

boolean SystemDUnit::StartServiceForking(boolean blocking)

{
    /* --- 2DO ---- */
    // Start a one shot service. 
    // First create an ordered dependency graph shuffling the various needs and wants lists

    // For each dependency, start according to type. 

    // Start this service: 
    // We don't bother with the service manager for a oneshot, since we are just executing the commands (once)
    // In this case, the advantage is the dependencies

    // Execute Start Pre:
    // wait for it?
    // Execute Start
    // wait for it?
    // Execute Start Post
    // wait for it?

}

boolean SystemDUnit::StartServiceDBus(boolean blocking)

{
    /* --- 2DO ---- */
    // Start a one shot service. 
    // First create an ordered dependency graph shuffling the various needs and wants lists

    // For each dependency, start. 

    // Start this service: 
    // We don't bother with the service manager for a oneshot, since we are just executing the commands (once)
    // In this case, the advantage is the dependencies

    // Execute Start Pre:
    // wait for it?
    // Execute Start
    // wait for it?
    // Execute Start Post
    // wait for it?

}

boolean SystemDUnit::StartServiceNotify(boolean blocking)

{
    /* --- 2DO ---- */
    // Start a one shot service. 
    // First create an ordered dependency graph shuffling the various needs and wants lists

    // For each dependency, start. 

    // Start this service: 
    // We don't bother with the service manager for a oneshot, since we are just executing the commands (once)
    // In this case, the advantage is the dependencies

    // Execute Start Pre:
    // wait for it?
    // Execute Start
    // wait for it?
    // Execute Start Post
    // wait for it?

}


boolean SystemDUnit::StopServiceOneShot(boolean blocking)

{
    /* --- 2DO ---- */
    // Stop a one shot service. 
    // We should have a saved depends list.

    // For each dependency, check stopped. If not, stop.  respecting timeouts

    // Stop this service: 
    // Execute Stop
    // wait for it?
    // Execute Stop Post
    // wait for it?

}


boolean SystemDUnit::StopServiceSimple(boolean blocking)

{
    /* --- 2DO ---- */
    // Stop a one shot service. 
    // We should have a saved depends list.

    // For each dependency, check stopped. If not, stop.  respecting timeouts

    // Stop this service: 
    // Execute Stop
    // wait for it?
    // Execute Stop Post
    // wait for it?

}


boolean SystemDUnit::StopServiceForking(boolean blocking)

{
    /* --- 2DO ---- */
    // Stop a one shot service. 
    // We should have a saved depends list.

    // For each dependency, check stopped. If not, stop via the apropiate type.  respecting timeouts

    // Stop this service: 
    // Execute Stop
    // wait for it?
    // Execute Stop Post
    // wait for it?

}


boolean SystemDUnit::StopServiceDBus(boolean blocking)

{
    /* --- 2DO ---- */
    // Stop a one shot service. 
    // We should have a saved depends list.

    // For each dependency, check stopped. If not, stop via the apropiate type.  respecting timeouts

    // Stop this service: 
    // Execute Stop
    // wait for it?
    // Execute Stop Post
    // wait for it?

}


boolean SystemDUnit::StopServiceNotify(boolean blocking)

{
    /* --- 2DO ---- */
    // Stop a notify service. 
    // We should have a saved depends list.

    // For each dependency, check stopped. If not, stop via the apropiate type.  respecting timeouts

    // Stop this service: 
    // Execute Stop
    // wait for it?
    // Execute Stop Post
    // wait for it?

}



boolean SystemDUnit::ReloadServiceOneShot(boolean blocking)

{
    /* --- 2DO ---- */
    // Reload a one shot service. 
    // We should have a saved depends list.

    // For each dependency, check reloadped. If not, reload.  respecting timeouts

    // Reload this service: 
    // Execute Reload
    // wait for it?
    // Execute Reload Post
    // wait for it?

}


boolean SystemDUnit::ReloadServiceSimple(boolean blocking)

{
    /* --- 2DO ---- */
    // Reload a one shot service. 
    // We should have a saved depends list.

    // For each dependency, check reloadped. If not, reload.  respecting timeouts

    // Reload this service: 
    // Execute Reload
    // wait for it?
    // Execute Reload Post
    // wait for it?

}


boolean SystemDUnit::ReloadServiceForking(boolean blocking)

{
    /* --- 2DO ---- */
    // Reload a one shot service. 
    // We should have a saved depends list.

    // For each dependency, check reloadped. If not, reload via the apropiate type.  respecting timeouts

    // Reload this service: 
    // Execute Reload
    // wait for it?
    // Execute Reload Post
    // wait for it?

}


boolean SystemDUnit::ReloadServiceDBus(boolean blocking)

{
    /* --- 2DO ---- */
    // Reload a one shot service. 
    // We should have a saved depends list.

    // For each dependency, check reloadped. If not, reload via the apropiate type.  respecting timeouts

    // Reload this service: 
    // Execute Reload
    // wait for it?
    // Execute Reload Post
    // wait for it?

}


boolean SystemDUnit::ReloadServiceNotify(boolean blocking)

{
    /* --- 2DO ---- */
    // Reload a notify service. 
    // We should have a saved depends list.

    // For each dependency, check reloadped. If not, reload via the apropiate type.  respecting timeouts

    // Reload this service: 
    // Execute Reload
    // wait for it?
    // Execute Reload Post
    // wait for it?

}


