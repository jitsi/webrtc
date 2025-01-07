#!/usr/bin/env bash

xcodebuild -create-xcframework -output build/ios/WebRTC.xcframework \
    -framework webrtc/ios/src/out/Release-ios-device-arm64/WebRTC.framework \
    -debug-symbols $(realpath webrtc/ios/src/out/Release-ios-device-arm64/WebRTC.dSYM) \
    -framework webrtc/ios/src/out/Release-ios-simulator-arm64/WebRTC.framework \
    -debug-symbols $(realpath webrtc/ios/src/out/Release-ios-simulator-arm64/WebRTC.dSYM) \
    -framework webrtc/ios/src/out/Release-macos-arm64/WebRTC.framework \
    -debug-symbols $(realpath webrtc/ios/src/out/Release-macos-arm64/WebRTC.dSYM)
