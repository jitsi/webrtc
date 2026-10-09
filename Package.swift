// swift-tools-version:5.7
import PackageDescription

let package = Package(
    name: "JitsiWebRTC",
    platforms: [.iOS(.v14), .macOS(.v13)],
    products: [
        .library(
            name: "WebRTC",
            targets: ["WebRTC"]),
    ],
    dependencies: [],
    targets: [
        .binaryTarget(
            name: "WebRTC",
            url: "https://github.com/jitsi/webrtc/releases/download/v155.0.0/WebRTC.xcframework.zip",
            checksum: "bfc104ed2d3afb4285f4502aa949f1b44a0435d6e29c377094b24ed421578f59"
        ),
    ]
)
