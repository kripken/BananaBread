#import <Cocoa/Cocoa.h>

@interface Development : NSView
+ (NSString*)svnRevisionOfPath:(NSString*)path;

// indicate that this is a non-standard version - typically either svn or recompiled
+ (void)updateWindow:(NSWindow*)window withType:(NSString*)type;
@end
