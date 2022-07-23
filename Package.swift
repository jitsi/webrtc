// swift-tools-version: 5.6
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "WebRTC",
    platforms: [
         .iOS(.v12)
	],
	products: [
		.library(
			name: "WebRTC",
			targets: ["WebRTCWrapper"]),
	],
	dependencies: [],
	targets: [
		.binaryTarget(
			name: "WebRTC",
			url: "https://github.com/Talk360/WebRTC/releases/download/100.0.0/WebRTC.xcframework-bitcode.zip",
			checksum: "30c683ceab974b23b4875d7f858da89863fb2f4f9328e87eef42ffffaeb29acc"
		),
		.target(
			name: "WebRTCWrapper",
			dependencies: [
				.target(name: "WebRTC")
			],
			path: "./Sources"
		)
	]
)
