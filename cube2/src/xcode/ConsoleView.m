#import "ConsoleView.h"

#define MAX_LINES 200
#define LINE_WIDTH 500
#define LINE_HEIGHT 12

/*
 * This VScroller sends a scrollToEnd message to the console based on whether you hit page/line down or not
 */
@interface VScroller : NSScroller {
}
@end
@implementation VScroller
- (NSScrollerPart)hitPart {
	ConsoleView *view =  (ConsoleView *)[(NSScrollView*)[self superview] documentView];
	NSScrollerPart part = [super hitPart];
	[view scrollToEnd:((part == NSScrollerIncrementPage) || (part == NSScrollerIncrementLine))];
	return part;
}
@end


@implementation ConsoleView

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
		array = [[[NSMutableArray alloc] init] retain];
		
		attr = [[NSDictionary dictionaryWithObjectsAndKeys:
			[NSFont userFontOfSize:(LINE_HEIGHT-2)], NSFontAttributeName, 
			[NSColor colorWithCalibratedRed:0.2 green:0.8 blue:0.2 alpha:1.0], NSForegroundColorAttributeName, 
			nil] retain];
    }
    return self;
}

- (void)awakeFromNib {
	[self scrollToEnd:YES];
	
	NSScroller *vscroll = [[VScroller alloc] init];
	[vscroll setControlSize:[[[self enclosingScrollView] verticalScroller] controlSize]];
	[[self enclosingScrollView] setVerticalScroller:vscroll];
}

- (void)dealloc {
	[attr release];
	[array release];
	[super dealloc];
}

- (BOOL)isFlipped { 
	return YES; 
}

- (void)drawRect:(NSRect)rect {	
	//draw the visible lines only
	int startLine = rect.origin.y/LINE_HEIGHT;
	int endLine = 1 + (rect.origin.y+rect.size.height)/LINE_HEIGHT;
	if(startLine < 0) startLine = 0;
	if(endLine > [array count]) endLine = [array count];
	int i;	
	for(i = startLine; i < endLine; i++) {
		NSString *str = [array objectAtIndex:i];
		[str drawAtPoint:NSMakePoint(2, i * LINE_HEIGHT) withAttributes:attr];
	}
}

- (void)scrollToEnd:(BOOL)enable {
	endScroll = enable;
}

- (void)appendLine:(NSString*)line {
	BOOL chop = [array count] > MAX_LINES;
	if(chop) {
		[array removeObjectAtIndex:0]; // limit the number of lines
	}
	[array addObject:line];	
	int i = [array count];
	[self setFrame:NSMakeRect(0, 0, LINE_WIDTH, i*LINE_HEIGHT)]; // increase the frame size	
		
	NSRect rect = NSMakeRect(0, (i-1)*LINE_HEIGHT, LINE_WIDTH, LINE_HEIGHT);
	if(endScroll) {
		// Scroll to the line just added
		if([self scrollRectToVisible:rect]) return;
	} else {
		// Lock the scrolling on the first visible line
		i = [self visibleRect].origin.y/LINE_HEIGHT;
		if(!chop) i++;
		if(i < 0) i = 0; 
		if(i > [array count]) i = [array count];
		NSRect vrect = NSMakeRect(0, (i-1)*LINE_HEIGHT, LINE_WIDTH, LINE_HEIGHT);
		if([self scrollRectToVisible:vrect]) return;
	}
	if(chop)
		[self setNeedsDisplay:YES];
	else
		[self setNeedsDisplayInRect:rect];
}

- (void)appendText:(NSString*)text {
	NSArray *lines = [text componentsSeparatedByString:@"\n"]; //@TODO assumes we get given lines rather than fragments...
	int i;
	for(i = 0; i < [lines count]; i++) {
		NSString *line = [lines objectAtIndex:i];
		if([line length] == 0) continue; //skip empty
		[self appendLine:line];
	}
}

- (BOOL)acceptsFirstResponder { 
	return YES; 
}

- (IBAction)delete:(id)sender {
	[array removeAllObjects];
	[self setFrame:NSMakeRect(0,0,0,0)];
}

@end
