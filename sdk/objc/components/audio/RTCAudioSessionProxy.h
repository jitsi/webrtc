/*
 *  Copyright 2016 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

#import "RTCMacros.h"

NS_ASSUME_NONNULL_BEGIN

RTC_OBJC_EXPORT
@interface RTC_OBJC_TYPE (RTCAudioSessionProxy) : NSObject
@property(nonatomic, readonly) AVAudioSession *native;

#pragma mark-

@property(copy, nullable) NSError *__nullable (^setActiveBlock)(BOOL active, AVAudioSessionSetActiveOptions options);
@property(copy, nullable) NSError *__nullable (^setCategoryBlock)(AVAudioSessionCategory category, AVAudioSessionCategoryOptions options);
@property(copy, nullable) NSError *__nullable (^setModeBlock)(NSString *mode);
@property(copy, nullable) NSError *__nullable (^setInputGainBlock)(float gain);
@property(copy, nullable) NSError *__nullable (^setPreferredSampleRateBlock)(double sampleRate);
@property(copy, nullable) NSError *__nullable (^setPreferredIOBufferDurationBlock)(NSTimeInterval duration);
@property(copy, nullable) NSError *__nullable (^setPreferredInputNumberOfChannelsBlock)(NSInteger count);
@property(copy, nullable) NSError *__nullable (^setPreferredOutputNumberOfChannelsBlock)(NSInteger count);
@property(copy, nullable) NSError *__nullable (^overrideOutputAudioPortBlock)(AVAudioSessionPortOverride portOverride);
@property(copy, nullable) NSError *__nullable (^setPreferredInputBlock)(AVAudioSessionPortDescription *inPort);
@property(copy, nullable) NSError *__nullable (^setInputDataSourceBlock)(AVAudioSessionDataSourceDescription *dataSource);
@property(copy, nullable) NSError *__nullable (^setOutputDataSourceBlock)(AVAudioSessionDataSourceDescription *dataSource);

#pragma mark-

- (instancetype)initWithAudioSession:(AVAudioSession*)audioSession;
- (instancetype)init NS_UNAVAILABLE;

#pragma mark-

@property(readonly) NSString *category;
@property(readonly) AVAudioSessionCategoryOptions categoryOptions;
@property(readonly) NSString *mode;
@property(readonly) BOOL secondaryAudioShouldBeSilencedHint;
@property(readonly) AVAudioSessionRouteDescription *currentRoute;
@property(readonly) NSInteger maximumInputNumberOfChannels;
@property(readonly) NSInteger maximumOutputNumberOfChannels;
@property(readonly) float inputGain;
@property(readonly) BOOL inputGainSettable;
@property(readonly) BOOL inputAvailable;
@property(readonly, nullable) NSArray<AVAudioSessionDataSourceDescription *> *inputDataSources;
@property(readonly, nullable) AVAudioSessionDataSourceDescription *inputDataSource;
@property(readonly, nullable) NSArray<AVAudioSessionDataSourceDescription *> *outputDataSources;
@property(readonly, nullable) AVAudioSessionDataSourceDescription *outputDataSource;
@property(readonly) double sampleRate;
@property(readonly) double preferredSampleRate;
@property(readonly) NSInteger inputNumberOfChannels;
@property(readonly) NSInteger outputNumberOfChannels;
@property(readonly) float outputVolume;
@property(readonly) NSTimeInterval inputLatency;
@property(readonly) NSTimeInterval outputLatency;
@property(readonly) NSTimeInterval IOBufferDuration;
@property(readonly) NSTimeInterval preferredIOBufferDuration;

#pragma mark-

- (BOOL)setActive:(BOOL)active withOptions:(AVAudioSessionSetActiveOptions)options error:(NSError **)outError;
- (BOOL)setCategory:(AVAudioSessionCategory)category
        withOptions:(AVAudioSessionCategoryOptions)options
              error:(NSError **)outError;
- (BOOL)setMode:(NSString *)mode error:(NSError **)outError;
- (BOOL)setInputGain:(float)gain error:(NSError **)outError;
- (BOOL)setPreferredSampleRate:(double)sampleRate error:(NSError **)outError;
- (BOOL)setPreferredIOBufferDuration:(NSTimeInterval)duration
                               error:(NSError **)outError;
- (BOOL)setPreferredInputNumberOfChannels:(NSInteger)count
                                    error:(NSError **)outError;
- (BOOL)setPreferredOutputNumberOfChannels:(NSInteger)count
                                     error:(NSError **)outError;
- (BOOL)overrideOutputAudioPort:(AVAudioSessionPortOverride)portOverride
                          error:(NSError **)outError;
- (BOOL)setPreferredInput:(AVAudioSessionPortDescription *)inPort
                    error:(NSError **)outError;
- (BOOL)setInputDataSource:(AVAudioSessionDataSourceDescription *)dataSource
                     error:(NSError **)outError;
- (BOOL)setOutputDataSource:(AVAudioSessionDataSourceDescription *)dataSource
                      error:(NSError **)outError;

@end

NS_ASSUME_NONNULL_END
