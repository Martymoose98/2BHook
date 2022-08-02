Some option explanations:

Process selection:
    Existing - select existing process from the list
    New - new process will be launched before injection
    Manual launch - after pressing 'Inject' button, injector will wait for target process startup
      
Images:
    List of images you want inject
    Add - add new image to the list. Drag'n'drop is also supported
    Remove - remove selected image
    Clear - clear image list
      
Advanced options:

    Injection type:
        Native inject - common approach using LoadLibraryW \ LdrLoadDll in newly created or existing thread
        Manual map - manual copying image data into target process memory without creating section object
        Kernel(New thread) - kernel mode ZwCreateThreadEx into LdrLoadDll. Uses driver
        Kernel(APC) - kernel mode APC into LdrLoadDll. Uses driver
        Kernel(Manual map) - kernel manual mapping. Uses driver
        
    Native Loader options:
        Unlink module - after injection, unlink module from InLoadOrderModuleList, InMemoryOrderModuleList, InInitializationOrderModuleList, HashLinks and LdrpModuleBaseAddressIndex.
        Erase PE - after injection, erase PE headers 
        Use existing thread - LoadLibrary and init routine will be executed in the context of random non-suspended thread.
            
    Manual map options:
        Add loader reference - Insert module record into InMemoryOrderModuleList/LdrpModuleBaseAddressIndex and HashLinks. Used to make module functions (e.g. GetModuleHandle, GetProcAddress) work with manually mapped image.
        Manually resolve imports - Image import and delayed import dlls will be also manually mapped instead of being loaded using LdrLoadDll.
        Wipe headers - Erase module header information after injection. Also affects manually mapped imports.
        Ignore TLS - Don't process image static TLS data and call TLS callbacks.
        No exception support - Don't create custom exception handlers that enable out-of-image exception support under DEP.
        Conceal memory - Make image memory visible as PAGE_NO_ACESS to memory query functions
        
    Command Line:
        Process command line arguments

    Init routine:
        If you are injecting native (not pure IL) image, this is name of exported function that will be called after injection is done. This export is called as void ( __stdcall* )(wchar_t*) function.
        If you are injecting pure managed image, this is name of public method that will be executed using ICLRRuntimeHost::ExecuteInDefaultAppDomain.

    Init argument:
        String that is passed into init routine
          
    Close after injection:
        Close injector after successful injection
        
    Inject delay:
        Delay before injection start
        
    Inject interval:
        Delay between each image
    
    Escalate handle access:
        Bypass handle acess filters. Driver is required
        
 Menu options:
    
    Profiles->Load - load injection profile
    Profiles->Save - save current settings into profile
    
    Tools->Eject modules - open module ejection dialog
    Tools->Protect self - make injector process protected (driver required)
        
Command line options:
	--load <profile_path> - start injector and load target profile specified by <profile_path>
	--run <profile_path> - immediately  execute profile specified by <profile_path> without GUI
		
Kernel injection methods require system running in Test mode.