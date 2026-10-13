# WebRTC

This repository contains the [WebRTC](https://webrtc.org) source code used to
build the libraries used by [react-native-webrtc](https://github.com/react-native-webrtc/react-native-webrtc).

For a given stable Chrome release (92 as an example) there will be 2 branches in this repository: M92-upstream and M92.

The former is an untouched branch off upstream, with the latter containing custom patches. All the patches we apply are
submitted upstream once testes, or abandoned.

## Builds

Builds are currently done by `@saghul` manually (since they require testing and potentially updating react-native-webrtc). Custom builds can be made with this [build script](tools/build-webrtc.py).

### Building for iOS

After having built the `WebRTC.xcframework.zip` artifact with the Python script some manual steps are necessary:

- Update the version number and path in `ios/JitsiWebRTC.podspec`
- Compute the SHA-256 of the built artifact (yes, the zip file) and update `Package.swift`
- Copy the `Package.swift` file to the `MXXX` release branch and add it to git
- Commit all changes
- Create a GH release with the following versioning: X.0.Y where X is the Chrome milestone number and Y is our build number starting at 0
    - The release should tag the milestone branch
- Upload the build artifact to the release
- Push the spec to CocoaPods: `pod trunk push ios/JitsiWebRTC.podspec`

### Building for Android

After having built the `webrtc-android.zip` artifact with the Python script some manual steps are necessary:

- Unzip the file and put it onto `android/libs` overwriting any existing file
- Adjust the version numbers in `android/build.gradle`
- Commit the result (via a PR)
- Run the `Release Android` workflow. It publishes `org.jitsi:webrtc:<version>` to Maven Central through the
  [Central Portal](https://central.sonatype.com) and releases the deployment automatically; nothing to click.

The workflow needs these repository secrets:

- `MAVEN_CENTRAL_USERNAME` / `MAVEN_CENTRAL_PASSWORD`: a Central Portal user token for the account owning `org.jitsi`
- `GPG_KEY` / `GPG_PW`: the ASCII-armored PGP private key used to sign the artifacts, and its passphrase

To try the packaging locally: `cd android && ./gradlew publishToMavenLocal` (unsigned unless `signingInMemoryKey`
and `signingInMemoryKeyPassword` are passed as Gradle properties).

## Versioning

Releases will follow the following versioning scheme: `Chrome Major Version`.`0`.`Release Number`

## CocoaPods

These builds are also published on CocoaPods with the name "JitsiWebRTC".

Starting with version 106 the builds don't contain bitcode (they do contain embedded dSYM slices).

## Swift Package Manager

As of release 106.0.0 a `Package.swift` file is provided for Swift Package Manager support.
