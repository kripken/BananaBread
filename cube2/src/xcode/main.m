#import <Cocoa/Cocoa.h>
#import "SDL.h"
#import "Launcher.h"

@interface SDLApplication : NSApplication
@end

@implementation SDLApplication

// Ensure SDL exits too
- (void)terminate:(id)sender
{
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
    
    [super terminate:sender];
}

// Prevent beeps from unhandled keys as a consequence of having enabled SDL_ENABLEAPPEVENTS
- (void)sendEvent:(NSEvent *)anEvent
{
	if([(Launcher*)[self delegate] gameRunning] && (NSKeyDown == [anEvent type] || NSKeyUp == [anEvent type]) ) {
		if( [anEvent modifierFlags] & NSCommandKeyMask ) 
			[super sendEvent: anEvent];
	} else 
		[super sendEvent: anEvent];
}
@end

#ifdef main
#  undef main
#endif

int main(int argc, char *argv[])
{
    BOOL server = NO;
    int i;
    for(i = 0; i < argc; i++) if(strcmp(argv[i], "-d") == 0) { server = YES; break; }
    if(server) return SDL_main(argc, (char**)argv);
    
	// neither of the followin is necessary as the principal class is set to SDLApplication in the plist 
	//[SDLApplication sharedApplication];
    //[SDLApplication poseAsClass:[NSApplication class]];
	
    return NSApplicationMain(argc,  (const char **)argv);
}
