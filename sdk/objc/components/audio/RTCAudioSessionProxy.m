/*
 *  Copyright 2016 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "RTCAudioSessionProxy.h"
#import "base/RTCLogging.h"

@interface RTC_OBJC_TYPE (RTCAudioSessionProxy)()
@property (nonatomic, strong) AVAudioSession *native;
@end

@implementation RTC_OBJC_TYPE (RTCAudioSessionProxy)

- (instancetype)init {
    return [self initWithAudioSession:[AVAudioSession sharedInstance]];
}

- (instancetype)initWithAudioSession:(AVAudioSession*)audioSession {
    if (self = [super init]) {
        _native = audioSession;
        RTCLog(@"RTC_OBJC_TYPE(RTCAudioSessionProxy) (%p): init.", self);
    }
    return self;
}

- (void)dealloc {
    RTCLog(@"RTC_OBJC_TYPE(RTCAudioSessionProxy) (%p): dealloc.", self);
}

#pragma mark -

- (BOOL)setActive:(BOOL)active withOptions:(AVAudioSessionSetActiveOptions)options error:(NSError **)outError
{
    if(self.setActiveBlock) {
        NSError* error = self.setActiveBlock(active, options);
        if (outError) {
            *outError = error;
        }
        return (error == NULL);
    }
    return [self.native setActive:active withOptions:options error:outError];
}

- (BOOL)setCategory:(NSString *)category
        withOptions:(AVAudioSessionCategoryOptions)options
              error:(NSError **)outError {
    if(self.setCategoryBlock) {
        NSError* error = self.setCategoryBlock(category, options);
        if (outError) {
            *outError = error;
        }
        return (error == NULL);
    }
    return [self.native setCategory:category withOptions:options error:outError];
}

- (BOOL)setMode:(NSString *)mode error:(NSError **)outError {
    if(self.setModeBlock) {
        NSError* error = self.setModeBlock(mode);
        if (outError) {
            *outError = error;
        }
        return (error == NULL);
    }
    return [self.native setMode:mode error:outError];
}

- (BOOL)setInputGain:(float)gain error:(NSError **)outError
{
    if(self.setInputGainBlock) {
        NSError* error = self.setInputGainBlock(gain);
        if (outError) {
            *outError = error;
        }
        return (error == NULL);
    }
    return [self.native setInputGain:gain error:outError];
}

- (BOOL)setPreferredSampleRate:(double)sampleRate error:(NSError **)outError
{
    if(self.setPreferredSampleRateBlock) {
        NSError* error = self.setPreferredSampleRateBlock(sampleRate);
        if (outError) {
            *outError = error;
        }
        return (error == NULL);
    }
    return [self.native setPreferredSampleRate:sampleRate error:outError];
}

- (BOOL)setPreferredIOBufferDuration:(NSTimeInterval)duration
                               error:(NSError **)outError
{
    if(self.setPreferredIOBufferDurationBlock) {
        NSError* error = self.setPreferredIOBufferDurationBlock(duration);
        if (outError) {
            *outError = error;
        }
        return (error == NULL);
    }
    return [self.native setPreferredIOBufferDuration:duration error:outError];
}

- (BOOL)setPreferredInputNumberOfChannels:(NSInteger)count
                                    error:(NSError **)outError
{
    if(self.setPreferredInputNumberOfChannelsBlock) {
        NSError* error = self.setPreferredInputNumberOfChannelsBlock(count);
        if (outError) {
            *outError = error;
        }
        return (error == NULL);
    }
    return [self.native setPreferredInputNumberOfChannels:count error:outError];
}

- (BOOL)setPreferredOutputNumberOfChannels:(NSInteger)count
                                     error:(NSError **)outError
{
    if(self.setPreferredOutputNumberOfChannelsBlock) {
        NSError* error = self.setPreferredOutputNumberOfChannelsBlock(count);
        if (outError) {
            *outError = error;
        }
        return (error == NULL);
    }
    return [self.native setPreferredOutputNumberOfChannels:count error:outError];
}

- (BOOL)overrideOutputAudioPort:(AVAudioSessionPortOverride)portOverride
                          error:(NSError **)outError
{
    if(self.overrideOutputAudioPortBlock) {
        NSError* error = self.overrideOutputAudioPortBlock(portOverride);
        if (outError) {
            *outError = error;
        }
        return (error == NULL);
    }
    return [self.native overrideOutputAudioPort:portOverride error:outError];
}

- (BOOL)setPreferredInput:(AVAudioSessionPortDescription *)inPort
                    error:(NSError **)outError
{
    if(self.setPreferredInputBlock) {
        NSError* error = self.setPreferredInputBlock(inPort);
        if (outError) {
            *outError = error;
        }
        return (error == NULL);
    }
    return [self.native setPreferredInput:inPort error:outError];
}

- (BOOL)setInputDataSource:(AVAudioSessionDataSourceDescription *)dataSource
                     error:(NSError **)outError
{
    if(self.setInputDataSourceBlock) {
        NSError* error = self.setInputDataSourceBlock(dataSource);
        if (outError) {
            *outError = error;
        }
        return (error == NULL);
    }
    return [self.native setInputDataSource:dataSource error:outError];
}

- (BOOL)setOutputDataSource:(AVAudioSessionDataSourceDescription *)dataSource
                      error:(NSError **)outError
{
    if(self.setOutputDataSourceBlock) {
        NSError* error = self.setOutputDataSourceBlock(dataSource);
        if (outError) {
            *outError = error;
        }
        return (error == NULL);
    }
    return [self.native setOutputDataSource:dataSource error:outError];
}

#pragma mark -

- (NSString *)category {
    return self.native.category;
}

- (AVAudioSessionCategoryOptions)categoryOptions {
    return self.native.categoryOptions;
}

- (NSString *)mode {
    return self.native.mode;
}

- (BOOL)secondaryAudioShouldBeSilencedHint {
    return self.native.secondaryAudioShouldBeSilencedHint;
}

- (AVAudioSessionRouteDescription *)currentRoute {
    return self.native.currentRoute;
}

- (NSInteger)maximumInputNumberOfChannels {
    return self.native.maximumInputNumberOfChannels;
}

- (NSInteger)maximumOutputNumberOfChannels {
    return self.native.maximumOutputNumberOfChannels;
}

- (float)inputGain {
    return self.native.inputGain;
}

- (BOOL)inputGainSettable {
    return self.native.inputGainSettable;
}

- (BOOL)inputAvailable {
    return self.native.inputAvailable;
}

- (NSArray<AVAudioSessionDataSourceDescription *> *)inputDataSources {
    return self.native.inputDataSources;
}

- (AVAudioSessionDataSourceDescription *)inputDataSource {
    return self.native.inputDataSource;
}

- (NSArray<AVAudioSessionDataSourceDescription *> *)outputDataSources {
    return self.native.outputDataSources;
}

- (AVAudioSessionDataSourceDescription *)outputDataSource {
    return self.native.outputDataSource;
}

- (double)sampleRate {
    return self.native.sampleRate;
}

- (double)preferredSampleRate {
    return self.native.preferredSampleRate;
}

- (NSInteger)inputNumberOfChannels {
    return self.native.inputNumberOfChannels;
}

- (NSInteger)outputNumberOfChannels {
    return self.native.outputNumberOfChannels;
}

- (float)outputVolume {
    return self.native.outputVolume;
}

- (NSTimeInterval)inputLatency {
    return self.native.inputLatency;
}

- (NSTimeInterval)outputLatency {
    return self.native.outputLatency;
}

- (NSTimeInterval)IOBufferDuration {
    return self.native.IOBufferDuration;
}

- (NSTimeInterval)preferredIOBufferDuration {
    return self.native.preferredIOBufferDuration;
}

@end
