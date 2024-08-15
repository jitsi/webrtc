// swift-tools-version:5.6
import PackageDescription

let package = Package(
    name: "JitsiWebRTC",
    platforms: [.iOS(.v12)],
    products: [
        .library(
            name: "WebRTC",
            targets: ["WebRTC"]),
    ],
    dependencies: [],
    targets: [
        .binaryTarget(
            name: "WebRTC",
            url: "https://github.com/jitsi/webrtc/releases/download/v124.0.1/WebRTC.xcframework.zip",
            checksum: "d2548ea930ee135ea8269041c34031f388014d2b2c8aa5f7a7cfb892b603c31c"
        ),
    ]
)
