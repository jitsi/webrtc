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
            url: "https://github.com/jitsi/webrtc/releases/download/v155.0.1/WebRTC.xcframework.zip",
            checksum: "5189cc5864d917400174f5bcee3a62f27c55524a28bccec783575d5efbda2ae5"
        ),
    ]
)
