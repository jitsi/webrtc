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
            url: "https://github.com/jitsi/webrtc/releases/download/v106.0.0/WebRTC.xcframework.zip",
            checksum: "f8cfe9bd5190f50080ae21105c5ed385998c9578cc06eb53bfdab91f1482a821"
        ),
    ]
)
