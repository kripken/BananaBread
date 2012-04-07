#import <Cocoa/Cocoa.h>

@class ConsoleView;

@interface Launcher : NSObject {
    IBOutlet NSWindow *window;
	
    //able to leave these disconnected
    IBOutlet NSView *view1; //Main
    IBOutlet NSView *view2; //Maps
    IBOutlet NSView *view3; //Keys
    IBOutlet NSView *view4; //Server
    IBOutlet NSView *view5; //EisenStern
    
	
    IBOutlet NSProgressIndicator *prog; //while scanning maps - it's there if you want to wire it up
    IBOutlet NSArrayController *maps;
    IBOutlet NSArrayController *keys;
    IBOutlet NSPopUpButton *resolutions;
    IBOutlet NSButton *multiplayer;
    IBOutlet ConsoleView *console;
@private
    NSMutableDictionary *toolBarItems;
    pid_t server;
    NSMutableDictionary *fileRoles;
    BOOL forcename;
    
    NSString *dataPath, *userPath;
    BOOL gamerunning;
}

- (IBAction)playAction:(id)sender;

- (IBAction)multiplayerAction:(id)sender;

- (IBAction)playRpg:(id)sender;

- (IBAction)playMap:(id)sender;

- (IBAction)openUserdir:(id)sender;

- (BOOL)serverRunning;

- (BOOL)gameRunning;

@end
