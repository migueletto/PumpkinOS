//
//  DeviceWindow.swift
//
//  Created by peter bohac on 4/16/24.
//

import SwiftUI

struct DeviceWindow: View {
    let window: WindowProvider.Window

    @Environment(Pit.self) private var pit
    @State private var title = "Untitled"
    @State private var mainWindow = Image(systemName: "xmark")
    @State private var isDragging = false

    var body: some View {
        VStack(spacing: 0) {
            mainWindow
                .focusable()
                .focusEffectDisabled()
                .gesture(
                    DragGesture(minimumDistance: 0)
                        .onChanged { event in
                            if isDragging {
                                pit.windowProvider.mouseMove(to: event.location)
                            } else {
                                pit.windowProvider.mouseDown(at: event.location)
                                isDragging = true
                            }
                        }
                        .onEnded { event in
                            pit.windowProvider.mouseUp(at: event.location)
                            isDragging = false
                        }
                )
                .onKeyPress(phases: .all) { keyPress in
                    pit.windowProvider.keyEvent(keyPress)
                    return .handled
                }
            Divider()
            HStack(spacing: 30) {
                Button("Home", systemImage: "house.circle") {
                    pit.windowProvider.keyEvent(.home)
                }
                Button("Menu", systemImage: "filemenu.and.selection") {
                    pit.windowProvider.keyEvent(.menu)
                }
            }
            .padding(5)
            .labelStyle(.iconOnly)
            .buttonStyle(.borderless)
            .font(.largeTitle)
        }
        .navigationTitle(title)
        .onAppear {
            title = window.title ?? title
            mainWindow = window.image
        }
        .onDisappear {
            Task {
                await pit.stop()
            }
        }
        .dropDestination(for: URL.self) { urls, _ in
            let items = urls.compactMap { $0.pathExtension == "prc" ? $0 : nil }
            guard items.isEmpty == false else { return false }
            return pit.installApps(items)
        }
        .task {
            let events = await pit.windowProvider.events.subscribe()
            for await event in events {
                switch event {
                case .setTitle(_, let title):
                    self.title = title
                case .draw(let window):
                    mainWindow = window.image
                case .createWindow, .destroyWindow:
                    break
                }
            }
        }
    }
}

extension WindowProvider.Window {
    var image: Image {
        let cgImage = buffer.withUnsafeMutableBytes { bufferPtr in
            let ctx = CGContext(
                data: bufferPtr.baseAddress,
                width: Int(width),
                height: Int(height),
                bitsPerComponent: 8,
                bytesPerRow: Int(width * pixelSize),
                space: CGColorSpace(name: CGColorSpace.sRGB)!,
                bitmapInfo: CGImageAlphaInfo.premultipliedFirst.rawValue
            )!
            return ctx.makeImage()!
        }
        return Image(cgImage, scale: 1, label: Text("Device Window"))
    }
}

#Preview {
    DeviceWindow(window: .empty)
        .environment(Pit())
}
