//
//  WindowProvider.swift
//
//  Created by peter bohac on 4/16/24.
//

import Foundation
import OSLog
import SwiftUI

final class WindowProvider: Sendable {
    enum Event: Sendable {
        case createWindow(Window)
        case destroyWindow(Window)
        case setTitle(Window, String)
        case draw(Window)
    }

    enum Input: Sendable {
        case keyDown(Int32)
        case keyUp(Int32)
        case mouseDown
        case mouseUp
        case mouseMove(x: Int32, y: Int32)
        case stop
    }

    enum Key: Sendable {
        case home
        case menu
    }

    let events: AsyncSharedStream<Event>
    private let publisher: AsyncStream<Event>.Continuation
    private var inputEvents: [Input] = []

    init() {
        let (events, publisher) = AsyncStream<Event>.makeStream()
        self.events = .init(events)
        self.publisher = publisher
        self.wp.data = Unmanaged.passUnretained(self).toOpaque()
    }

    @MainActor
    func reset() {
        inputEvents.removeAll()
    }

    @MainActor
    func keyEvent(_ key: Key) {
        let code = switch key {
        case .home: WINDOW_KEY_HOME
        case .menu: WINDOW_KEY_F5
        }
        inputEvents.append(.keyDown(code))
        inputEvents.append(.keyUp(code))
    }

    @MainActor
    func keyEvent(_ keyPress: KeyPress) {
        let code: Int32 = switch keyPress.key {
        case .tab: 9
        case .home: WINDOW_KEY_HOME
        case .leftArrow: WINDOW_KEY_LEFT
        case .upArrow: WINDOW_KEY_UP
        case .rightArrow: WINDOW_KEY_RIGHT
        case .downArrow: WINDOW_KEY_DOWN
        default:
            switch keyPress.key.character {
            case "\u{7F}": 8    // Delete key
            case "\r": 13       // Return key
            case "\u{F704}": WINDOW_KEY_F1
            case "\u{F705}": WINDOW_KEY_F2
            case "\u{F706}": WINDOW_KEY_F3
            case "\u{F707}": WINDOW_KEY_F4
            case "\u{F708}": WINDOW_KEY_F5
            default: Int32(keyPress.key.character.asciiValue ?? 0)
            }
        }
        switch keyPress.phase {
        case .down, .repeat:
            inputEvents.append(.keyDown(code))
        case .up:
            inputEvents.append(.keyUp(code))
        default:
            break
        }
    }

    @MainActor
    func mouseDown(at location: CGPoint) {
        inputEvents.append(.mouseMove(x: Int32(location.x), y: Int32(location.y)))
        inputEvents.append(.mouseDown)
    }

    @MainActor
    func mouseMove(to location: CGPoint) {
        inputEvents.append(.mouseMove(x: Int32(location.x), y: Int32(location.y)))
    }

    @MainActor
    func mouseUp(at location: CGPoint) {
        inputEvents.append(.mouseMove(x: Int32(location.x), y: Int32(location.y)))
        inputEvents.append(.mouseUp)
    }

    @MainActor
    func stop() {
        inputEvents.append(.stop)
    }

    private static func create(
        encoding: Int32,
        widthPtr: UnsafeMutablePointer<Int32>?,
        heightPtr: UnsafeMutablePointer<Int32>?,
        xfactor: Int32,
        yfactor: Int32,
        rotate: Int32,
        fullscreen: Int32,
        software: Int32,
        dataPtr: UnsafeMutableRawPointer?
    ) -> UnsafeMutablePointer<window_t?>? {
        guard let dataPtr else { return nil }
        let windowProvider: WindowProvider = Unmanaged.fromOpaque(dataPtr).takeUnretainedValue()
        let width = widthPtr?.pointee ?? 0
        let height = heightPtr?.pointee ?? 0
        let window = Window(
            encoding: encoding,
            width: width,
            height: height,
            provider: windowProvider
        )
        Logger.wp.debug("Create window: \(window.id); encoding=\(encoding), width=\(width), height=\(height)")
        windowProvider.publisher.yield(.createWindow(window))
        return Unmanaged.passRetained(window).toOpaque().assumingMemoryBound(to: window_t?.self)
    }

    private static func destroy(
        windowPtr: UnsafeMutablePointer<window_t?>?
    ) -> Int32 {
        guard let windowPtr else { return -1 }
        let window: Window = Unmanaged.fromOpaque(UnsafeRawPointer(windowPtr)).takeRetainedValue()
        Logger.wp.debug("Destroy window: \(window.id)")
        window.provider?.publisher.yield(.destroyWindow(window))
        return 0
    }

    private static func title(
        windowPtr: UnsafeMutablePointer<window_t?>?,
        titlePtr: UnsafeMutablePointer<CChar>?
    ) {
        guard let windowPtr, let titlePtr else { return }
        let window: Window = Unmanaged.fromOpaque(UnsafeRawPointer(windowPtr)).takeUnretainedValue()
        let title = String(cString: titlePtr)
        Logger.wp.debug("Title window: \(window.id); '\(title)'")
        window.title = title
        window.provider?.publisher.yield(.setTitle(window, title))
    }

    private static func createTexture(
        windowPtr: UnsafeMutablePointer<window_t?>?,
        width: Int32,
        height: Int32
    ) -> OpaquePointer? {
        guard let windowPtr else { return nil }
        let window: Window = Unmanaged.fromOpaque(UnsafeRawPointer(windowPtr)).takeUnretainedValue()
        let texture = Texture(width: width, height: height, pixelSize: window.pixelSize)
        Logger.wp.debug("Create texture: \(texture.id)")
        return .init(Unmanaged.passRetained(texture).toOpaque())
    }

    private static func destroyTexture(
        windowPtr: UnsafeMutablePointer<window_t?>?,
        texturePtr: OpaquePointer?
    ) -> Int32 {
        guard let texturePtr else { return 0 }
        let texture: Texture = Unmanaged.fromOpaque(UnsafeRawPointer(texturePtr)).takeRetainedValue()
        Logger.wp.debug("Destroy texture: \(texture.id)")
        return 0
    }

    private static func updateTexture(
        windowPtr: UnsafeMutablePointer<window_t?>?,
        texturePtr: OpaquePointer?,
        rawPtr: UnsafeMutablePointer<UInt8>?
    ) -> Int32 {
        guard let texturePtr, let rawPtr else { return -1 }
        let texture: Texture = Unmanaged.fromOpaque(UnsafeRawPointer(texturePtr)).takeUnretainedValue()
        Logger.wp.debug("Update texture from raw: \(texture.id)")
        texture.buffer.withUnsafeMutableBytes { bufferPtr in
            let src = UnsafeRawBufferPointer(start: rawPtr, count: texture.size)
            bufferPtr.copyMemory(from: src)
        }
        return 0
    }

    private static func updateTexture(
        windowPtr: UnsafeMutablePointer<window_t?>?,
        texturePtr: OpaquePointer?,
        srcPtr: UnsafeMutablePointer<UInt8>?,
        tx: Int32,
        ty: Int32,
        w: Int32,
        h: Int32
    ) -> Int32 {
        guard let windowPtr, let texturePtr, let srcPtr else { return -1 }
        let window: Window = Unmanaged.fromOpaque(UnsafeRawPointer(windowPtr)).takeUnretainedValue()
        let texture: Texture = Unmanaged.fromOpaque(UnsafeRawPointer(texturePtr)).takeUnretainedValue()
        Logger.wp.debug("Update texture from src: \(texture.id)")
        let len = Int(w * window.pixelSize)
        let pitch = Int(texture.width * window.pixelSize)
        texture.buffer.withUnsafeMutableBytes { bufferPtr in
            let start = Int((ty * texture.width + tx) * window.pixelSize)
            let destAddr = bufferPtr.baseAddress!.advanced(by: start)
            var dest = UnsafeMutableRawBufferPointer(start: destAddr, count: bufferPtr.count - start)
            let srcAddr = srcPtr.advanced(by: start)
            var src = UnsafeRawBufferPointer(start: srcAddr, count: len)
            for _ in 0 ..< h {
                dest.copyMemory(from: src)
                let destAddr = dest.baseAddress!.advanced(by: pitch)
                dest = UnsafeMutableRawBufferPointer(start: destAddr, count: dest.count - pitch)
                let srcAddr = src.baseAddress!.advanced(by: pitch)
                src = UnsafeRawBufferPointer(start: srcAddr, count: len)
            }
        }
        return 0
    }

    private static func drawTexture(
        windowPtr: UnsafeMutablePointer<window_t?>?,
        texturePtr: OpaquePointer?,
        tx: Int32,
        ty: Int32,
        w: Int32,
        h: Int32,
        x: Int32,
        y: Int32
    ) -> Int32 {
        guard let windowPtr, let texturePtr else { return -1 }
        let window: Window = Unmanaged.fromOpaque(UnsafeRawPointer(windowPtr)).takeUnretainedValue()
        let texture: Texture = Unmanaged.fromOpaque(UnsafeRawPointer(texturePtr)).takeUnretainedValue()
        Logger.wp.debug("Draw texture rect: \(texture.id)")
        var (tx, ty, w, h, x, y) = (tx, ty, w, h, x, y)
        if x < 0 {
            let diff = 0 - x
            tx += diff
            w -= diff
            x += diff
        }
        if y < 0 {
            let diff = 0 - y
            ty += diff
            h -= diff
            y += diff
        }
        if (x + w) > window.width {
            w = window.width - x
        }
        if (y + h) > window.height {
            h = window.height - y
        }
        if w <= 0 || h <= 0 || x >= window.width || y >= window.height {
            // nothing to render
            return 0
        }

        let len = Int(w * window.pixelSize)
        let windowPitch = Int(window.width * window.pixelSize)
        let texturePitch = Int(texture.width * window.pixelSize)
        window.buffer.withUnsafeMutableBytes { windowBuffer in
            texture.buffer.withUnsafeBytes { textureBuffer in
                var start = Int((y * window.width + x) * window.pixelSize)
                let destAddr = windowBuffer.baseAddress!.advanced(by: start)
                var dest = UnsafeMutableRawBufferPointer(start: destAddr, count: windowBuffer.count - start)
                start = Int((ty * texture.width + tx) * window.pixelSize)
                let srcAddr = textureBuffer.baseAddress!.advanced(by: start)
                var src = UnsafeRawBufferPointer(start: srcAddr, count: len)
                for _ in 0 ..< h {
                    dest.copyMemory(from: src)
                    let destAddr = dest.baseAddress!.advanced(by: windowPitch)
                    dest = UnsafeMutableRawBufferPointer(start: destAddr, count: max(dest.count - windowPitch, 0))
                    let srcAddr = src.baseAddress!.advanced(by: texturePitch)
                    src = UnsafeRawBufferPointer(start: srcAddr, count: len)
                }
            }
        }
        window.provider?.publisher.yield(.draw(window))
        return 0
    }

    private static func event2(
        windowPtr: UnsafeMutablePointer<window_t?>?,
        wait: Int32,
        arg1Ptr: UnsafeMutablePointer<Int32>?,
        arg2Ptr: UnsafeMutablePointer<Int32>?
    ) -> Int32 {
        guard
            let window = windowPtr.map({ Unmanaged.fromOpaque(UnsafeRawPointer($0)).takeUnretainedValue() as Window }),
            let isEmpty = window.provider?.inputEvents.isEmpty,
            isEmpty == false,
            let event = window.provider?.inputEvents.removeFirst()
        else { return 0 }

        switch event {
        case .keyDown(let code):
            Logger.wp.debug("Window key down: \(code)")
            arg1Ptr?.pointee = code
            arg2Ptr?.pointee = 0
            return WINDOW_KEYDOWN
        case .keyUp(let code):
            Logger.wp.debug("Window key up: \(code)")
            arg1Ptr?.pointee = code
            arg2Ptr?.pointee = 0
            return WINDOW_KEYUP
        case .mouseDown:
            arg1Ptr?.pointee = 1
            arg2Ptr?.pointee = 0
            return WINDOW_BUTTONDOWN
        case .mouseMove(let x, let y):
            arg1Ptr?.pointee = x
            arg2Ptr?.pointee = y
            return WINDOW_MOTION
        case .mouseUp:
            arg1Ptr?.pointee = 1
            arg2Ptr?.pointee = 0
            return WINDOW_BUTTONUP
        case .stop:
            return -1
        }
    }

    final class Window: Codable, Hashable, Sendable {
        let id: UUID
        let encoding: Int32
        let width: Int32
        let height: Int32
        var provider: WindowProvider?
        var title: String?
        let size: Int
        var buffer: [UInt8]

        var pixelSize: Int32 { 4 }

        init(encoding: Int32, width: Int32, height: Int32, provider: WindowProvider? = nil) {
            assert(encoding == ENC_RGBA, "Only RGBA is currently supported")
            self.id = UUID()
            self.encoding = encoding
            self.width = width
            self.height = height
            self.provider = provider
            self.size = Int(width * height) * 4
            self.buffer = Array(repeating: 0, count: size)
        }

        private enum CodingKeys: CodingKey {
            case id, encoding, width, height, title, size, buffer
        }

        static func == (lhs: Window, rhs: Window) -> Bool {
            lhs.id == rhs.id
        }

        func hash(into hasher: inout Hasher) {
            hasher.combine(id)
        }

        static let empty = Window(encoding: ENC_RGBA, width: 320, height: 320)
    }

    final class Texture: Sendable {
        let id = UUID()
        let width: Int32
        let height: Int32
        let size: Int
        var buffer: [UInt8]

        init(width: Int32, height: Int32, pixelSize: Int32) {
            self.width = width
            self.height = height
            self.size = Int(width * height * pixelSize)
            self.buffer = Array(repeating: 0, count: size)
        }
    }

    var wp = window_provider_t(
        create: { WindowProvider.create(encoding: $0, widthPtr: $1, heightPtr: $2, xfactor: $3, yfactor: $4, rotate: $5, fullscreen: $6, software: $7, dataPtr: $8) },
        event: nil,
        destroy: { WindowProvider.destroy(windowPtr: $0) },
        erase: nil,
        render: nil,
        background: nil,
        create_texture: { WindowProvider.createTexture(windowPtr: $0, width: $1, height: $2) },
        destroy_texture: { WindowProvider.destroyTexture(windowPtr: $0, texturePtr: $1) },
        update_texture: { WindowProvider.updateTexture(windowPtr: $0, texturePtr: $1, rawPtr: $2) },
        draw_texture: nil,
        status: nil,
        title: { WindowProvider.title(windowPtr: $0, titlePtr: $1) },
        clipboard: nil,
        event2: { WindowProvider.event2(windowPtr: $0, wait: $1, arg1Ptr: $2, arg2Ptr: $3) },
        update: nil,
        draw_texture_rect: { WindowProvider.drawTexture(windowPtr: $0, texturePtr: $1, tx: $2, ty: $3, w: $4, h: $5, x: $6, y: $7) },
        update_texture_rect: { WindowProvider.updateTexture(windowPtr: $0, texturePtr: $1, srcPtr: $2, tx: $3, ty: $4, w: $5, h: $6) },
        move: nil,
        average: nil,
        data: nil
    )
}
