#import <Cocoa/Cocoa.h>

/*
 * A basic console view
 */

@interface ConsoleView : NSView {
@private
	NSMutableArray *array;
	NSDictionary *attr;
	BOOL endScroll;
}
- (void)scrollToEnd:(BOOL)enable;

/*
 * Append the given line
 */
- (void)appendLine:(NSString*)line;

/*
 * Append multiple lines, skip blank ones
 */
- (void)appendText:(NSString*)text;

@end
