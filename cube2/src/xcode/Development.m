#import "Development.h"

@implementation Development

+ (NSString*)svnRevisionOfPath:(NSString*)path {
    NSString *revision = nil;
    @try {
        NSPipe *pipe = [NSPipe pipe];
        NSTask *task = [[[NSTask alloc] init] autorelease];
        [task setLaunchPath: @"/usr/bin/svn"];
        [task setArguments:[NSArray arrayWithObjects: @"info", path, nil]];
        [task setStandardOutput:pipe];
        [task launch];
        NSData *data = [[pipe fileHandleForReading] readDataToEndOfFile];
        NSString *string = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] autorelease];
        NSString *prefix = @"Revision: ";
        NSEnumerator *lines = [[string componentsSeparatedByString:@"\n"] objectEnumerator];
        NSString *line;
        while((line = [lines nextObject])) {
            if([line hasPrefix:prefix]) {
                revision = [line substringFromIndex:[prefix length]];
                break;
            }
        }
    } @catch(...) {
    }
    return revision;
}

+ (void)updateWindow:(NSWindow*)window withType:(NSString*)type {
   
    NSString *title = [window title];
    [window setTitle:[NSString stringWithFormat:@"%@: %@", title, type]];
    
    //(credit) titlebar height: http://www.borkware.com/quickies/one?topic=NSWindow
    NSRect frame = NSMakeRect (0, 0, 100, 100);
    NSRect contentRect = [NSWindow contentRectForFrameRect:frame styleMask:[window styleMask]];
    const float height = (frame.size.height - contentRect.size.height);
    
    //(credit) omni
    NSView *borderView = [window valueForKey:@"borderView"];
    if(borderView) {
        NSRect borderBounds = [borderView bounds];
        NSRect contructionFrame = NSMakeRect(NSMinX(borderBounds), NSMaxY(borderBounds) - height, NSWidth(borderBounds), height);
        Development *contructionView = [[Development alloc] initWithFrame:contructionFrame];
        [contructionView setAutoresizingMask:NSViewWidthSizable|NSViewMinYMargin];
        [borderView addSubview:contructionView positioned:NSWindowBelow relativeTo:nil];
        [contructionView release];
    }
}

- (BOOL)isOpaque { return NO; }

- (void)drawRect:(NSRect)rect {
    NSRect bounds = [self bounds];
    const float stripeWidth = 10.0f;
    float height = bounds.size.height;
    CGContextRef ctx = [[NSGraphicsContext currentContext] graphicsPort];
    CGContextSaveGState(ctx);
	CGContextSetBlendMode(ctx, kCGBlendModeDarken); // 10.4+
	CGContextSetAlpha(ctx, 0.3);
	CGContextBeginTransparencyLayer(ctx, NULL);
    [[NSColor redColor] setFill];
    NSRectFill(bounds);
    [[NSColor yellowColor] setFill];
    CGPoint p = CGPointMake(bounds.origin.x - bounds.size.height, bounds.origin.y); // start far enough to the left that we'll cover the title area    
    while (p.x <= NSMaxX(bounds)) {
		CGContextMoveToPoint(ctx, p.x, p.y);
		CGContextAddLineToPoint(ctx, p.x + height, p.y + height);
		CGContextAddLineToPoint(ctx, p.x + height + stripeWidth, p.y + height);
		CGContextAddLineToPoint(ctx, p.x + stripeWidth, p.y);
		CGContextClosePath(ctx);
		p.x += 2*stripeWidth;
    }
    CGContextFillPath(ctx);
	CGContextEndTransparencyLayer(ctx);
    CGContextRestoreGState(ctx);
}

- (NSView *)hitTest:(NSPoint)aPoint { return nil; }

@end
