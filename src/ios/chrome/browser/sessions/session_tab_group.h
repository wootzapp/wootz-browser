// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_SESSIONS_SESSION_TAB_GROUP_H_
#define IOS_CHROME_BROWSER_SESSIONS_SESSION_TAB_GROUP_H_

#import <Foundation/Foundation.h>

// NSCoding-compliant class used to serialize tab groups.
@interface SessionTabGroup : NSObject <NSCoding>

- (instancetype)initWithRangeStart:(NSInteger)rangeStart
                        rangeCount:(NSInteger)rangeCount
                             title:(NSString*)title
                           colorId:(NSInteger)colorId NS_DESIGNATED_INITIALIZER;
- (instancetype)init NS_UNAVAILABLE;

@property(nonatomic, assign) NSInteger rangeStart;
@property(nonatomic, assign) NSInteger rangeCount;
@property(nonatomic, copy) NSString* title;
@property(nonatomic, assign) NSInteger colorId;

@end

#endif  // IOS_CHROME_BROWSER_SESSIONS_SESSION_TAB_GROUP_H_
