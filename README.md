# WebRTC

This repository contains the [WebRTC](https://webrtc.org) source code used to
build the libraries used by [react-native-webrtc](https://github.com/react-native-webrtc/react-native-webrtc).

For a given stable Chrome release (92 as an example) there will be 2 branches in this repository: M92-upstream and M92.

The former is an untouched branch off upstream, with the latter containing custom patches. All the patches we apply are
submitted upstream once testes, or abandoned.

## Builds

Builds are currently done by `@saghul` manually (since they require testing and potentially updating react-native-webrtc). Custom builds can be made with this [build script](https://github.com/react-native-webrtc/react-native-webrtc/blob/master/tools/build-webrtc.py).

## Versioning

Releases will follow the following versioning scheme: `Chrome Major Version`.`0`.`Release Number`

## CocoaPods

These builds are also published on CocoaPods with the name "JitsiWebRTC".

Starting with version 106 the builds don't contain bitcode (they do contain embedded dSYM slices).

## Swift Package Manager

As of release 106.0.0 a `Package.swift` file is provided for Swift Package Manager support.
