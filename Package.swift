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
            url: "https://github.com/jitsi/webrtc/releases/download/v111.0.1/WebRTC.xcframework.zip",
            checksum: "594aa24f68b171ee856202cded6c679bf9ddbc3bc115283172e6fff119f81c71"
        ),
    ]
)
