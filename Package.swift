// swift-tools-version:5.7
import PackageDescription

let package = Package(
    name: "JitsiWebRTC",
    platforms: [.iOS(.v12), .macOS(.v13)],
    products: [
        .library(
            name: "WebRTC",
            targets: ["WebRTC"]),
    ],
    dependencies: [],
    targets: [
        .binaryTarget(
            name: "WebRTC",
            url: "https://github.com/jitsi/webrtc/releases/download/v124.0.2/WebRTC.xcframework.zip",
            checksum: "a5fc15bd2547e9e282e3756885c76fe78a223082e0f6a767a03791ea525f0e2e"
        ),
    ]
)
