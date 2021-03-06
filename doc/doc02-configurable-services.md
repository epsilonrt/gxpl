Using configurable devices  
> Copyright (c) 2005, Gerald R Duprey Jr.  
> Copyright (c) 2015, epsilonRT                  

## Introduction 

gxPL fully supports the xPL device configuration
protocol.  The protocol allows your xPL program to be configured remotely by
applications like DCM or xPLHAL and preserve those settings locally,
automatically reloading the settings/configuration the next time your
application starts.

Ideally, you would want to expose all configurable elements of your
application to gxPL.  Doing so grants you customization and persistance
with no additional effort on your behalf and provides a means for users to
configure your program without having to have a seperate configuration program
or confusing command startup options.


## Integrating configuration into gxPL applications

If you have an existing gxPL application, you can get automatic support
for a configurable instance ID, heartbeat interval, filters and groups with
only one line of code changed.  Replace an existing:

<pre class="fragment">
myDevice = gxPLAppAddDevice(myApp, myVendor, myDevice, myInstance);
</pre>

with

<pre class="fragment">
myDevice = gxPLAppAddConfigurableDevice(myApp, myVendor, myDevice, configFilename);
</pre>

That's it! configFileName is the file configuration data it stored in and
automatically reloaded when the app restarts.  Often, it's the device name
with a .xpl extension (i.e. *cdp1802-clock might* use *clock.xpl*).  You can pass
NULL as the configFile name and while configuration will still work, the
results will not persist across application restartts.

If you are wondering where the instanceID went, it's created as a unique value
automatically as needed.  If there is no configuration, the a new unique
identifier is assigned.  This is meant only to be a placeholder until the end
user first configures your application/device.  Once they do, the newly
assigned instance ID will be stored along with the other configurables for the
device and restored when next started.

It's a VERY good idea to put your programs version number into the device
your program uses.  This version number will show up in the xPL configuration
programs and it allows a user to see what version all their apps are from a
common location.  In general, the version number should be plain (i.e. 1.1 is
preferred over V1.1), though that does not have to be the rule.  You do want the 
version number to be connected automatically to your programs version number so 
you do not have to remember to seek the xPL device version value out and change
 it (it's easy to forget).  For example


You should insure that every message you send that is supposed to come from
this device is sent with the

<pre class="fragment">
gxPLDeviceMessageSend(myDevice, myMessage);
</pre>

instead of the more generic *gxPLAppBroadcastMessage()*.  The reason is that if the
services instance ID is changed by the user and you continue to use the
simpler *gxPLAppBroadcastMessage()* on *gxPLMessage* instances you created earlier, your
messages source fields may no longer match what the services identifiers are
now.  *gxPLDeviceMessageSend()* makes sure (and corrects, if needed) they
always match.


## Exposing your own configurables to xPL

While letting the user customize the basic information about your device is a
very good thing, integrating any configuration elements your program has into
your xPL configurable device is where the real benefits are.

First, some background: xPL configuration items are pretty much only text.
You of course can represent numbers, booleans, etc, but you do need to be
prepared to shuttle values between text and whatever native format your
configuration data is in.

An xPL configuration item (all called a "configurable") has a unique name
within each device.  It can be set as a mandatory configuration item that can
only set set once (rarely used), a mandatory configuration item that can be
later reconfigured or an optional configuration item that can be filled in or
left blank and later reconfigured.  

In addition, configuration items can be defined to only hold one value or to
hold multiple values.  For example, a configurable item like "debugMode" would
tend to have only one value -- true or false (or on or off, etc).  But a more
complex item like "allowedUserName" may be able to have many values (i.e. a
list of each username that is allowed).  When you define a configurable, you
tell it what the maximum number of values you'll permit to be set into that
configurable is.


Now, some details:

1.  Configurable definitions are preserved, along with their settings, in the
    services configuration file.  This means you want to check to see if the
    device is configured or not before adding the configurable definitions.
    If it's already configured, then you do not need to (and should not) add
    your configurables in.  Here's a code sample:

    <pre class="fragment">
    myDevice = gxPLAppAddConfigurableDevice(myApp, "myVendor", "myDevice", "test.xpl");
    
    if (!gxPLDeviceIsConfigured(myDevice) {
      
      gxPLDeviceConfigItemAdd(myDevice, "debugMode", gxPLConfigReconf, 1);
      gxPLDeviceConfigValueSet(myDevice, "debugMode", "false");
    }
    </pre>

    In this example, after creating the configurable device (which will restore
    it's settings, if possible, from the passed configuration file), you test to
    see if the device is configured.  If so, then the config file was successfully
    restored.  If false though, either there was an error or there is no config
    file to restore from.  So in that case, you need to "prime the pump".

    In the case that it's unconfigured, we define a configurable named
    "debugMode".  It's set to be a mandatory but reconfigurable item and to have
    only one possible value.

    We also install an intial value to serve as a "first time" default. You do
    not need to put code to set your defaults here and then for your program as
    the next steps will insure that your application will automatically be
    configured, either from your config file or from your defaults.

2.  In general, it's best to wrap up all the code that extracts configurable
    values from the device and installs them where your program needs them in
    a single method that can be invoked from anywhere.  Done this way, you'll
    be able to invoke this from both your initialization code and from an event
    handler that is invoked to alert you when the device has new or changed
    configuration values.  In this case, it might look like this:

    <pre class="fragment">
    static void prvParseConfigValues(gxPLDevice * device) {
      
     // Extract the value, if any, for a configurable named "debugMode"
     char * debugFlag = gxPLDeviceConfigValueGet(device, "debugMode");
     if (debugFlag != NULL) {
       
       // Do a case insensitive compare to see if it's true or not
       debugMode = (strcasecmp(debugFlag, "true") == 0);
     }
    }
    </pre>

    It can be that easy.  Or it could be more complex if you need to compare
    the configurables value with a current value and only apply a change if the
    configurable is different.  For example, if you had a configurable that
    held the name of a file your program wrote to, you would not want to
    blindly apply a change (resulting in closing/opening the file) if there was
    no real change in that file name configurables value.

    Note that the assumption is there is a global variable called debugMode
    that is set to 0 or 1 to indicate if the program should print debugging
    messages.  Also, the strcasecmp is provided in xPL and does
    pretty much what it says.  While xPL identifiers in general should be lower
    case, unless case really matters, you should try to be "generous" when
    looking at configurable values (or received xPL message values, for that
    sake).  Some platforms have a strcmpignorecase() function, some don't, so
    you can choose to use it if there is or use the xPL one and be fairly sure
    it'll work anywhere xPL will work.

    You should place a call to the parser right after the code that checks for
    and initializes the configurables.  That way, it'll get invoked whether the
    configuration file was read (to parse it's values into a useful form) or
    not (so it then parses the default values into your program).

    For example:

    <pre class="fragment">
    // Create the device
    myDevice = gxPLAppAddConfigurableDevice(myApp, "myVendor", "myDevice", "test.xpl");
    
    if (!gxPLDeviceIsConfigured(myDevice) {
      
      gxPLDeviceConfigItemAdd(myDevice, "debugMode", gxPLConfigReconf, 1);
      gxPLDeviceConfigValueSet(myDevice, "debugMode", "false");
    }

    // Parse configuration
    prvParseConfigValues(myDevice);
    </pre>


3.  You'll probably want to know when some outside force (a configuration
    program like DCM or xPLHAL) changes your configuration values.  This is
    pretty easy to do -- just add a listener and a method to receive events.

    For example:

    <pre class="fragment">
    static void prvConfigChanged(gxPLDevice * device, void * userData) {
      
      prvParseConfigValues(device);
    }
    </pre>

    This handler will just invoke the same parser we used earlier to parse
    things after the device was created.

    To register that handler, use gxPLaddServiceConfigChangedListener() right after you
    finish the initial parsing of config values.  Like this:

    <pre class="fragment">
    // Create the device
    myDevice = gxPLAppAddConfigurableDevice(myApp, "myVendor", "myDevice", "test.xpl");
    
    if (!gxPLDeviceIsConfigured(myDevice) {
      
      gxPLDeviceConfigItemAdd(myDevice, "debugMode", gxPLConfigReconf, 1);
      gxPLDeviceConfigValueSet(myDevice, "debugMode", "false");
    }

    // Parse configuration
    prvParseConfigValues(myDevice);

    // Add a listener for configuration changes
    gxPLDeviceConfigListenerAdd(myDevice, prvConfigChanged, NULL);
    </pre>


4.  At this point, you are fully configuable ready.  You've created a
    configurable device, reloaded your previous configuration (or setup the
    initial configurables with defaults), parsed them so your program can use
    them, writen a handler to pickup changes to the configuration as they
    happen and registered that handler.

    All that is left to do is enable the device (which should not be done
    until all these steps are complete).  The device will come to life and
    automatic configuration should be working.

    A working example of this is in the examples/ directory called
    gxpl-config-clock.c -- a working xPL time source that allows you to configure
    it's instance ID, heartbeat interval and how often time messages are sent
    out.

    You can also check out the updated gxpl-logger.c that supports two
    configurable items and does check to determine if there really is a change
    to a config value before actually applying the change (in this case, that
    the file name the log should go to really has changed before we close the
    old and open the new).


NOTE: Once a configurable device is enabled, you cannot change any aspect of
the services configuration.  That includes adding and removing configurables
or adding/changing/clearing configurable values or changing the config file
used to store config values.  When the device is disabled, these features can
be used again.  Access to the configurables and their values is always
allowed.  If you attempt to change these items while the device is enabled,
the changes will be ignored.

Remember - the configuration process is driven by the end user -- not your
program.  Trying to fight it will result in configuration programs (like DCM
and xPLHAL) giving people confusing/misleading/wrong data and users being
frustrated that what they expect doesn't work.


## Overview of the xPL device configuration process

This is an overview of how configuration works in xPL.  For the most part,
gxPL does this all for you, so this is mostly to understand what is going
on under the hood.  

The general xPL configuration process goes like this:

1. First time a configurable device starts up, it sends out a
   heartbeat type of config.basic or config.app.  Unlike normal
   heartbeats, this heartbeat is sent every minute, regardless of the
   eventual heartbeat rate of the xPL device.

2. When an xPL configuration program hears of your application, it
   sends a config.list request to it.  You application then needs to
   send a list of configurable items your device support.  This list
   includes the name of the item, what type of configurable it is
   (mandatory one-time configuration, mandatory but reconfigurable as
   often as needed or optional).  It also indicates if each
   configuration item holds a single value or multiple values and if
   multiple values, what the maximum number of values is.

3. The configuration program receives the config.list and
   parsed/validates and enumerates the configurable items.  If there
   are no errors, it then sends a config.current to your program,
   asking for the current version of each configurable item.

4. You application receives the config.current request and sends back
   a config.current response with each configurable item and it's
   current value in the message.  If a configuration item has >1
   value, then the configurables name appears multiple times -- once
   per value -- with a discrete value for it.

5. The configuration program receives the current settings and is then
   ready to let the user edit the values.  When the user completes, the
   new values are sent to the program in a config.response packet.

6. The program receives the config.response packet and installs the new
   values.  Once the program validates and installs the values, it tries to
   save them locally (if allowed to) and then marks itself as configured.  At
   that point, the program sends a config.end message out to signal the end of
   configuration, then sends a hbeat.app out with the (possibly new) instance
   ID and (possibly new) heartbeat interval.  At this point, initial
   configuration is complete.

7. When your program starts next time, it should load its previously stored
   configuration values up.  If it has previous values and it's able to
   install them, it goes immediately to hbeat.app mode.  If there are no
   stored values, it's back to config.app heartbeats and basically back to
   step #1.

8. When a configuration program hears for a new application that is not
   sending a config.app heartbeat, it should consider the device as
   unconfigurable.  But it should send a config.list to the device.  If the
   device truly cannot be configured, it'll not reply to such a message.
   However, if the device can be configured, it will reply and the
   configuration program then knows it has a configurable application on it's
   hands.  It will generally then request current values and make itself ready
   to edit those values, if the user wants to.

9. Whenever the program receives a new configuration update from a
   configuration manager program, it needs to pay special attention to the
   passed instance ID and heartbeat interval.  If either have changed, then an
   extra step in the configuration is done.  First, the program sends a
   hbeat.end message with the old instance ID and heartbeat interval.  The new
   instance ID/heartbeat interval are installed and then the program should
   send a new hbeat.app out using the new instance ID or interval.

   If the instance ID or heartbeat interval change, it is NOT valid to just 
   start broadcasting with the new instance ID/interval.

   Any change in configuration should be written to a local file, if allowed, 
   as soon as it's installed for future restoration.

