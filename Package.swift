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
            url: "https://github.com/jitsi/webrtc/releases/download/v118.0.0/WebRTC.xcframework.zip",
            checksum: "e01960ca52aca9acff3ae712391a5eee4393fd5a32accab0c3801f330615ea1b"
        ),
    ]
)
