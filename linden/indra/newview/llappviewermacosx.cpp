/**
 * @file llappviewermacosx.cpp
 * @brief The LLAppViewerMacOSX class definitions
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */ 

#include "llviewerprecompiledheaders.h"

#if !defined LL_DARWIN
	#error "Use only with Mac OS X"
#endif

#include "llappviewermacosx.h"
#include "llcommandlineparser.h"

#include "llmemtype.h"

#include "llviewernetwork.h"
#include "llviewercontrol.h"
#include "llmd5.h"
#include "llurlsimstring.h"
#include "llfloaterworldmap.h"
#include "llurldispatcher.h"
#include <Carbon/Carbon.h>
#include "lldir.h"
#include <signal.h>
class LLWebBrowserCtrl;		// for LLURLDispatcher

namespace 
{
	// The command line args stored.
	// They are not used immediately by the app.
	int gArgC;
	char** gArgV;
	
	bool sCrashReporterIsRunning = false;
	
	OSErr AEQuitHandler(const AppleEvent *messagein, AppleEvent *reply, long refIn)
	{
		OSErr result = noErr;
		
		LLAppViewer::instance()->userQuit();
		
		return(result);
	}
}

int main( int argc, char **argv ) 
{
	LLMemType mt1(LLMemType::MTYPE_STARTUP);

#if LL_SOLARIS && defined(__sparc)
	asm ("ta\t6");		 // NOTE:  Make sure memory alignment is enforced on SPARC
#endif

	// Set the working dir to <bundle>/Contents/Resources
	if (chdir(gDirUtilp->getAppRODataDir().c_str()) == -1)
	{
		llwarns << "Could not change directory to "
				<< gDirUtilp->getAppRODataDir() << ": " << strerror(errno)
				<< llendl;
	}

	LLAppViewerMacOSX* viewer_app_ptr = new LLAppViewerMacOSX();

	viewer_app_ptr->setErrorHandler(LLAppViewer::handleViewerCrash);

	// Store off the command line args for use later.
	gArgC = argc;
	gArgV = argv;
	
	bool ok = viewer_app_ptr->init();
	if(!ok)
	{
		llwarns << "Application init failed." << llendl;
		return -1;
	}

		// Run the application main loop
	if(!LLApp::isQuitting()) 
	{
		viewer_app_ptr->mainLoop();
	}

	if (!LLApp::isError())
	{
		//
		// We don't want to do cleanup here if the error handler got called -
		// the assumption is that the error handler is responsible for doing
		// app cleanup if there was a problem.
		//
		viewer_app_ptr->cleanup();
	}
	delete viewer_app_ptr;
	viewer_app_ptr = NULL;

	return 0;
}

LLAppViewerMacOSX::LLAppViewerMacOSX()
{
}

LLAppViewerMacOSX::~LLAppViewerMacOSX()
{
}

bool LLAppViewerMacOSX::init()
{
	return LLAppViewer::init();
}

// MacOSX may add and addition command line arguement for the process serial number.
// The option takes a form like '-psn_0_12345'. The following method should be able to recognize
// and either ignore or return a pair of values for the option.
// look for this method to be added to the parser in parseAndStoreResults.
std::pair<std::string, std::string> parse_psn(const std::string& s)
{
    if (s.find("-psn_") == 0) 
	{
		// *FIX:Mani Not sure that the value makes sense.
		// fix it once the actual -psn_XXX syntax is known.
		return std::make_pair("psn", s.substr(5));
    }
	else 
	{
        return std::make_pair(std::string(), std::string());
    }
}

bool LLAppViewerMacOSX::initParseCommandLine(LLCommandLineParser& clp)
{
	// The next two lines add the support for parsing the mac -psn_XXX arg.
	clp.addOptionDesc("psn", NULL, 1, "MacOSX process serial number");
	clp.setCustomParser(parse_psn);
	
	// First parse the command line, not often used on the mac.
	if(clp.parseCommandLine(gArgC, gArgV) == false)
	{
		return false;
	}
    
    // Now read in the args from arguments txt.
    // Succesive calls to clp.parse... will NOT override earlier 
    // options. 
    const char* filename = "arguments.txt";
	llifstream ifs(filename, llifstream::binary);
	if (!ifs.is_open())
	{
		llwarns << "Unable to open file" << filename << llendl;
		return false;
	}
	
	if(clp.parseCommandLineFile(ifs) == false)
	{
		return false;
	}
	
	// Get the user's preferred language string based on the Mac OS localization mechanism.
	// To add a new localization:
		// go to the "Resources" section of the project
		// get info on "language.txt"
		// in the "General" tab, click the "Add Localization" button
		// create a new localization for the language you're adding
		// set the contents of the new localization of the file to the string corresponding to our localization
		//   (i.e. "en-us", "ja", etc.  Use the existing ones as a guide.)
	CFURLRef url = CFBundleCopyResourceURL(CFBundleGetMainBundle(), CFSTR("language"), CFSTR("txt"), NULL);
	char path[MAX_PATH];
	if(CFURLGetFileSystemRepresentation(url, false, (UInt8 *)path, sizeof(path)))
	{
		std::string lang;
		if(_read_file_into_string(lang, path))		/* Flawfinder: ignore*/
		{
            LLControlVariable* c = gSavedSettings.getControl("SystemLanguage");
            if(c)
            {
                c->setValue(lang, false);
            }
		}
	}
	CFRelease(url);
	
    return true;
}

// *FIX:Mani It would be nice to provide a clean interface to get the
// default_unix_signal_handler for the LLApp class.
extern void default_unix_signal_handler(int, siginfo_t *, void *);
bool LLAppViewerMacOSX::restoreErrorTrap()
{
	// This method intends to reinstate signal handlers.
	// *NOTE:Mani It was found that the first execution of a shader was overriding
	// our initial signal handlers somehow.
	// This method will be called (at least) once per mainloop execution.
	// *NOTE:Mani The signals used below are copied over from the 
	// setup_signals() func in LLApp.cpp
	// LLApp could use some way of overriding that func, but for this viewer
	// fix I opt to avoid affecting the server code.
	
	// Set up signal handlers that may result in program termination
	//
	struct sigaction act;
	struct sigaction old_act;
	act.sa_sigaction = default_unix_signal_handler;
	sigemptyset( &act.sa_mask );
	act.sa_flags = SA_SIGINFO;
	
	unsigned int reset_count = 0;
	
#define SET_SIG(S) 	sigaction(SIGABRT, &act, &old_act); \
					if((unsigned int)act.sa_sigaction != (unsigned int) old_act.sa_sigaction) \
						++reset_count;
	// Synchronous signals
	SET_SIG(SIGABRT)
	SET_SIG(SIGALRM)
	SET_SIG(SIGBUS)
	SET_SIG(SIGFPE)
	SET_SIG(SIGHUP) 
	SET_SIG(SIGILL)
	SET_SIG(SIGPIPE)
	SET_SIG(SIGSEGV)
	SET_SIG(SIGSYS)
	
	SET_SIG(LL_HEARTBEAT_SIGNAL)
	SET_SIG(LL_SMACKDOWN_SIGNAL)
	
	// Asynchronous signals that are normally ignored
	SET_SIG(SIGCHLD)
	SET_SIG(SIGUSR2)
	
	// Asynchronous signals that result in attempted graceful exit
	SET_SIG(SIGHUP)
	SET_SIG(SIGTERM)
	SET_SIG(SIGINT)
	
	// Asynchronous signals that result in core
	SET_SIG(SIGQUIT)	
#undef SET_SIG
	
	return reset_count == 0;
}

void LLAppViewerMacOSX::handleSyncCrashTrace()
{
	// do nothing
}

static OSStatus CarbonEventHandler(EventHandlerCallRef inHandlerCallRef, 
								   EventRef inEvent, 
								   void* inUserData)
{
    ProcessSerialNumber psn;
	
    GetEventParameter(inEvent, 
					  kEventParamProcessID, 
					  typeProcessSerialNumber, 
					  NULL, 
					  sizeof(psn), 
					  NULL, 
					  &psn);
	
    if( GetEventKind(inEvent) == kEventAppTerminated ) 
	{
		Boolean matching_psn = FALSE;	
		OSErr os_result = SameProcess(&psn, (ProcessSerialNumber*)inUserData, &matching_psn);
		if(os_result >= 0 && matching_psn)
		{
			sCrashReporterIsRunning = false;
		}
    }
    return noErr;
}

void LLAppViewerMacOSX::handleCrashReporting(bool reportFreeze)
{
	// This used to use fork&exec, but is switched to LSOpenApplication to 
	// Make sure the crash reporter launches in front of the SL window.
	
	std::string command_str;
	//command_str = "open Second Life.app/Contents/Resources/mac-crash-logger.app";
	command_str = "mac-crash-logger.app/Contents/MacOS/mac-crash-logger";
	
	FSRef appRef;
	Boolean isDir = 0;
	OSStatus os_result = FSPathMakeRef((UInt8*)command_str.c_str(),
									   &appRef,
									   &isDir);
	if(os_result >= 0)
	{
		LSApplicationParameters appParams;
		memset(&appParams, 0, sizeof(appParams));
	 	appParams.version = 0;
		appParams.flags = kLSLaunchNoParams | kLSLaunchStartClassic;
		appParams.application = &appRef;
		
		if(reportFreeze)
		{
			// Make sure freeze reporting launches the crash logger synchronously, lest 
			// Log files get changed by SL while the logger is running.
		
			// *NOTE:Mani A better way - make a copy of the data that the crash reporter will send
			// and let SL go about its business. This way makes the mac work like windows and linux
			// and is the smallest patch for the issue. 
			sCrashReporterIsRunning = false;
			ProcessSerialNumber o_psn;

			static EventHandlerRef sCarbonEventsRef = NULL;
			static const EventTypeSpec kEvents[] = 
			{
				{ kEventClassApplication, kEventAppTerminated }
			};
			
			// Install the handler to detect crash logger termination
			InstallEventHandler(GetApplicationEventTarget(), 
								(EventHandlerUPP) CarbonEventHandler,
								GetEventTypeCount(kEvents),
								kEvents,
								&o_psn,
								&sCarbonEventsRef
								);
			
			// Remove, temporarily the quit handler - which has *crash* behavior before 
			// the mainloop gets running!
			AERemoveEventHandler(kCoreEventClass, 
								 kAEQuitApplication, 
								 NewAEEventHandlerUPP(AEQuitHandler),
								 false);

			// Launch the crash reporter.
			os_result = LSOpenApplication(&appParams, &o_psn);
			
			if(os_result >= 0)
			{
				sCrashReporterIsRunning = true;
			}

			while(sCrashReporterIsRunning) 
			{
				RunApplicationEventLoop();
			}
            
			// Re-install the apps quit handler.
			AEInstallEventHandler(kCoreEventClass, 
								  kAEQuitApplication, 
								  NewAEEventHandlerUPP(AEQuitHandler),
								  0, 
								  false);
			
			// Remove the crash reporter quit handler.
			RemoveEventHandler(sCarbonEventsRef);
		}
		else
		{
			appParams.flags |= kLSLaunchAsync;
			clear_signals();

			ProcessSerialNumber o_psn;
			os_result = LSOpenApplication(&appParams, &o_psn);
		}
		
	}

	if(!reportFreeze)
	{
		_exit(1);
	}
	
	// TODO:palmer REMOVE THIS VERY SOON.  THIS WILL NOT BE IN VIEWER 2.0
	// Remove the crash stack log from previous executions.
	// Since we've started logging a new instance of the app, we can assume 
	// The old crash stack is invalid for the next crash report.
	char path[MAX_PATH];		
	FSRef folder;
	if(FSFindFolder(kUserDomain, kLogsFolderType, false, &folder) == noErr)
	{
		// folder is an FSRef to ~/Library/Logs/
		if(FSRefMakePath(&folder, (UInt8*)&path, sizeof(path)) == noErr)
		{
			std::string pathname = std::string(path) + std::string("/CrashReporter/");
			std::string mask = "Second Life*";
			std::string file_name;
			while(gDirUtilp->getNextFileInDir(pathname, mask, file_name, false))
			{
				LLFile::remove(pathname + file_name);
			}
		}
	}
	
}

std::string LLAppViewerMacOSX::generateSerialNumber()
{
	char serial_md5[MD5HEX_STR_SIZE];		// Flawfinder: ignore
	serial_md5[0] = 0;

	// JC: Sample code from http://developer.apple.com/technotes/tn/tn1103.html
	CFStringRef serialNumber = NULL;
	io_service_t    platformExpert = IOServiceGetMatchingService(kIOMasterPortDefault,
																 IOServiceMatching("IOPlatformExpertDevice"));
	if (platformExpert) {
		serialNumber = (CFStringRef) IORegistryEntryCreateCFProperty(platformExpert,
																	 CFSTR(kIOPlatformSerialNumberKey),
																	 kCFAllocatorDefault, 0);		
		IOObjectRelease(platformExpert);
	}
	
	if (serialNumber)
	{
		char buffer[MAX_STRING];		// Flawfinder: ignore
		if (CFStringGetCString(serialNumber, buffer, MAX_STRING, kCFStringEncodingASCII))
		{
			LLMD5 md5( (unsigned char*)buffer );
			md5.hex_digest(serial_md5);
		}
		CFRelease(serialNumber);
	}

	return serial_md5;
}

OSErr AEGURLHandler(const AppleEvent *messagein, AppleEvent *reply, long refIn)
{
	OSErr result = noErr;
	DescType actualType;
	char buffer[1024];		// Flawfinder: ignore
	Size size;
	
	result = AEGetParamPtr (
		messagein,
		keyDirectObject,
		typeCString,
		&actualType,
		(Ptr)buffer,
		sizeof(buffer),
		&size);	
	
	if(result == noErr)
	{
		std::string url = buffer;
		
		// Safari 3.2 silently mangles secondlife:///app/ URLs into
		// secondlife:/app/ (only one leading slash).
		// Fix them up to meet the URL specification. JC
		const std::string prefix = "secondlife:/app/";
		std::string test_prefix = url.substr(0, prefix.length());
		LLStringUtil::toLower(test_prefix);
		if (test_prefix == prefix)
		{
			url.replace(0, prefix.length(), "secondlife:///app/");
		}
		
		LLMediaCtrl* web = NULL;
		const bool trusted_browser = false;
		LLURLDispatcher::dispatch(url, web, trusted_browser);
	}
	
	return(result);
}

OSStatus simpleDialogHandler(EventHandlerCallRef handler, EventRef event, void *userdata)
{
	OSStatus result = eventNotHandledErr;
	OSStatus err;
	UInt32 evtClass = GetEventClass(event);
	UInt32 evtKind = GetEventKind(event);
	WindowRef window = (WindowRef)userdata;
	
	if((evtClass == kEventClassCommand) && (evtKind == kEventCommandProcess))
	{
		HICommand cmd;
		err = GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, sizeof(cmd), NULL, &cmd);
		
		if(err == noErr)
		{
			switch(cmd.commandID)
			{
				case kHICommandOK:
					QuitAppModalLoopForWindow(window);
					result = noErr;
				break;
				
				case kHICommandCancel:
					QuitAppModalLoopForWindow(window);
					result = userCanceledErr;
				break;
			}
		}
	}
	
	return(result);
}

void init_apple_menu(const char* product)
{
	// Load up a proper menu bar.
	{
		OSStatus err;
		IBNibRef nib = NULL;
		// NOTE: DO NOT translate or brand this string.  It's an internal name in the .nib file, and MUST match exactly.
		err = CreateNibReference(CFSTR("SecondLife"), &nib);
		
		if(err == noErr)
		{
			// NOTE: DO NOT translate or brand this string.  It's an internal name in the .nib file, and MUST match exactly.
			SetMenuBarFromNib(nib, CFSTR("MenuBar"));
		}

		if(nib != NULL)
		{
			DisposeNibReference(nib);
		}
	}
	
	// Install a handler for 'gurl' AppleEvents.  This is how secondlife:// URLs get passed to the viewer.
	
	if(AEInstallEventHandler('GURL', 'GURL', NewAEEventHandlerUPP(AEGURLHandler),0, false) != noErr)
	{
		// Couldn't install AppleEvent handler.  This error shouldn't be fatal.
		llinfos << "Couldn't install 'GURL' AppleEvent handler.  Continuing..." << llendl;
	}

	// Install a handler for 'quit' AppleEvents.  This makes quitting the application from the dock work.
	if(AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(AEQuitHandler),0, false) != noErr)
	{
		// Couldn't install AppleEvent handler.  This error shouldn't be fatal.
		llinfos << "Couldn't install Quit AppleEvent handler.  Continuing..." << llendl;
	}
}
