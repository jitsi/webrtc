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
            url: "https://github.com/jitsi/webrtc/releases/download/v124.0.0/WebRTC.xcframework.zip",
            checksum: "a7e0105c2646bd5d395dab6b99a5c0d072d9cc38009908815e60f5855ea440ee"
        ),
    ]
)
