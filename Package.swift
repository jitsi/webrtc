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
            url: "https://github.com/jitsi/webrtc/releases/download/v111.0.0/WebRTC.xcframework.zip",
            checksum: "db2c7124ced756d339a650169394e4bcd9c89a6945542ead029763097e3c094c"
        ),
    ]
)
