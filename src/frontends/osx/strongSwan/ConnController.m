/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#import "ConnController.h"

@interface ConnController ()

@end

@implementation ConnController

- (void)moveWindowToActiveSpace
{
	NSInteger behavior, old;

	/* seems that NSWindowCollectionBehaviorMoveToActiveSpace does not work
	 * when a window is opened a second time. Fix that by changing the
	 * behavior forth and back. */
	old = behavior = [[self window] collectionBehavior];
	behavior &= ~NSWindowCollectionBehaviorMoveToActiveSpace;
	behavior |= NSWindowCollectionBehaviorCanJoinAllSpaces;

	[[self window] setCollectionBehavior: behavior];
	[[self window] setCollectionBehavior: old];

	[NSApp activateIgnoringOtherApps:YES];
}

- (IBAction)saveConnEditor:(id)sender
{
	[NSApp stopModal];
}

- (IBAction)cancelConnEditor:(id)sender
{
    [NSApp abortModal];
}

//  DEFAULT_PROPOSAL="aes256gcm16-sha256-";}

- (NSMutableDictionary*)createConnection
{
	NSMutableDictionary *conn = nil;

	[[self window] setTitle:@"Add new connection"];
	[name setStringValue:@""];
	[server setStringValue:@""];
	[user setStringValue:@""];
    [ike_dh setStringValue:@"x25519"];
    [ike_qs setStringValue:@"sikep503"];
    [esp_dh setStringValue:@"x25519"];
    [esp_qs setStringValue:@"newhope512cca"];
	[ok setEnabled:FALSE];
	[self moveWindowToActiveSpace];
	if ([NSApp runModalForWindow: [self window]] == NSRunStoppedResponse)
	{
		conn = [NSMutableDictionary dictionaryWithObjectsAndKeys:
				[name stringValue], @"name",
				[server stringValue], @"server",
				[user stringValue], @"username",
                [ike_dh titleOfSelectedItem], @"ike_dh",
                [ike_qs titleOfSelectedItem], @"ike_qs",
                [esp_dh titleOfSelectedItem], @"esp_dh",
                [esp_qs titleOfSelectedItem], @"esp_qs",
				nil];
	}
	[[self window] orderOut: self];
	return conn;
}

- (bool)editConnection:(NSMutableDictionary*)conn
{
	bool edited = NO;

	[[self window] setTitle:@"Edit connection"];
	[name setStringValue:[conn objectForKey:@"name"]];
	[server setStringValue:[conn objectForKey:@"server"]];
	[user setStringValue:[conn objectForKey:@"username"]];
    [ike_dh setTitle:[conn objectForKey:@"ike_dh"]];
    [ike_qs setTitle:[conn objectForKey:@"ike_qs"]];
    [esp_dh setTitle:[conn objectForKey:@"esp_dh"]];
    [esp_qs setTitle:[conn objectForKey:@"esp_qs"]];
	[ok setEnabled:TRUE];
	[self moveWindowToActiveSpace];
	if ([NSApp runModalForWindow: [self window]] == NSRunStoppedResponse)
	{
		[conn setObject:[name stringValue] forKey:@"name"];
		[conn setObject:[server stringValue] forKey:@"server"];
		[conn setObject:[user stringValue] forKey:@"username"];
        [conn setObject:[ike_dh titleOfSelectedItem] forKey:@"ike_dh"];
        [conn setObject:[ike_qs titleOfSelectedItem] forKey:@"ike_qs"];
        [conn setObject:[esp_dh titleOfSelectedItem] forKey:@"esp_dh"];
        [conn setObject:[esp_qs titleOfSelectedItem] forKey:@"esp_qs"];
		edited = YES;
	}
	[[self window] orderOut: self];
	return edited;
}

- (void)controlTextDidChange:(NSNotification *)notification
{
	[ok setEnabled:
	 [[name stringValue] length] &&
	 [[server stringValue] length] &&
	 [[user stringValue] length]];
}

- (IBAction)popupBoxChanged:(id)sender {

    [self controlTextDidChange:nil];
}
@end
