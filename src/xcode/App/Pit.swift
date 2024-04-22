//
//  Pit.swift
//
//  Created by peter bohac on 4/15/24.
//

@preconcurrency import Foundation
import Observation
import OSLog

@Observable
final class Pit: Sendable {
    enum LogLevel: Int, CaseIterable, Codable {
        case error, info, debug
    }

    struct ModuleLogLevel: Codable, Identifiable {
        let id: UUID
        var level: LogLevel
        var module: String
        init(level: LogLevel, module: String) {
            self.id = UUID()
            self.level = level
            self.module = module
        }
    }

    private(set) var logId = UUID()
    private(set) var logLines: AsyncStream<LogLine>?
    @MainActor private(set) var running = false
    let windowProvider = WindowProvider()

    private var mainLoopTask: Task<Void, Never>?

    enum Constants {
        static let tempScript = URL.temporaryDirectory.appending(path: "main.lua")
        static let libraryDir = URL.libraryDirectory.appending(path: Bundle.main.bundleIdentifier!)
        static let vfsUrl = libraryDir.appending(path: "vfs/") // The trailing / is necessary
        static let appInstall = vfsUrl.appending(path: "app_install")
        static let cachesDir =  URL.cachesDirectory.appending(path: Bundle.main.bundleIdentifier!)
        static let logUrl = cachesDir.appending(path: "pumpkin.log")
    }

    init() {
        self.createVFSDirectory()
    }

    deinit {
        mainLoopTask?.cancel()
    }

    @MainActor
    func main(
        globalLogLevel: LogLevel = .info,
        moduleLogging: [ModuleLogLevel] = [],
        width: Int = 320,
        height: Int = 320
    ) {
        guard
            let templateURL = Bundle.main.url(forResource: "main", withExtension: "lua"),
            let template = try? String(contentsOf: templateURL)
        else {
            Logger.default.critical("Failed to get script template")
            return
        }
        let script = template
            .replacingOccurrences(of: "{width}", with: "\(width)")
            .replacingOccurrences(of: "{height}", with: "\(height)")
        do {
            try script.write(to: Constants.tempScript, atomically: true, encoding: .utf8)
        } catch {
            Logger.default.critical("Failed to write lua script: \(error.log, privacy: .public)")
            return
        }
        windowProvider.reset()
        running = true
        createLogStream()
        mainLoopTask = Task.detached(priority: .userInitiated) { [weak self] in
            guard let self else { return }
            let logging = ["-d", "\(globalLogLevel.rawValue)"] + moduleLogging.flatMap {
                ["-d", "\($0.level.rawValue):\($0.module)"]
            }
            let args = ["app"] + logging + ["-f", Constants.logUrl.path, "-s", "libscriptlua.so", Constants.tempScript.path]
            var cargs = args.map { strdup($0) }
            defer { cargs.forEach { free($0) } }
            let result = pit_main(
                Int32(args.count),
                &cargs,
                { Pit.mainCallback(enginePtr: $0, data: $1) },
                Unmanaged.passUnretained(self).toOpaque()
            )
            if result != 0 {
                Logger.default.critical("pit_main returned error status: \(result)")
            }
            Task { @MainActor [weak self] in
                self?.running = false
            }
        }
    }

    @MainActor
    func stop() async {
        windowProvider.stop()
        try? await Task.sleep(for: .milliseconds(500))
        mainLoopTask?.cancel()
        mainLoopTask = nil
        running = false
    }

    @MainActor
    func reset() {
        guard running == false else { return }
        if FileManager.default.fileExists(atPath: Constants.libraryDir.path) {
            do {
                try FileManager.default.removeItem(at: Constants.libraryDir)
            } catch {
                Logger.default.critical("Unable to delete library folder: \(error.log, privacy: .public)")
            }
        }
        createVFSDirectory()
    }

    @MainActor
    func installApps(_ urls: [URL]) -> Bool {
        do {
            for source in urls {
                let filename = source.lastPathComponent
                let destination = Constants.appInstall.appending(path: filename)
                if FileManager.default.fileExists(atPath: destination.path) {
                    try FileManager.default.removeItem(at: destination)
                }
                try FileManager.default.copyItem(at: source, to: destination)
            }
        } catch {
            Logger.default.critical("Failed to copy app for isntall: \(error.log, privacy: .public)")
            return false
        }
        windowProvider.deployApps()
        return true
    }

    private static func mainCallback(enginePtr: Int32, data: UnsafeMutableRawPointer?) {
        guard let data else { return }
        let instance: Pit = Unmanaged.fromOpaque(data).takeUnretainedValue()
        instance.mountVFS()
        instance.registerWindowProvider(enginePtr: enginePtr)
    }

    private func mountVFS() {
        let local = strdup(Constants.vfsUrl.path())
        let virtual = strdup("/")
        if vfs_local_mount(local, virtual) != 0 {
            Logger.default.critical("Failed to mount virtual file system")
        }
        free(virtual)
        free(local)
    }

    private func registerWindowProvider(enginePtr: Int32) {
        let cStr = strdup(WINDOW_PROVIDER)
        if script_set_pointer(enginePtr, cStr, &windowProvider.wp) != 0 {
            Logger.default.critical("Failed to register window provider")
        }
        free(cStr)
    }

    @MainActor
    private func createLogStream() {
        self.logLines = .init { publisher in
            try? FileManager.default.createDirectory(at: Constants.cachesDir, withIntermediateDirectories: true)
            try? "".write(to: Constants.logUrl, atomically: true, encoding: .utf8)
            let fileHandle = try! FileHandle(forReadingFrom: Constants.logUrl) // swiftlint:disable:this force_try
            let source = DispatchSource.makeFileSystemObjectSource(
                fileDescriptor: fileHandle.fileDescriptor,
                eventMask: .extend,
                queue: .global()
            )
            let logger = Logger.pit
            source.setEventHandler {
                guard
                    let string = String(data: fileHandle.availableData, encoding: .utf8)?.trimmingCharacters(in: .whitespacesAndNewlines),
                    string.isEmpty == false
                else { return }
                string.components(separatedBy: .newlines)
                    .map(LogLine.init)
                    .forEach { logLine in
                        switch logLine.level {
                        case .trace: logger.debug("\(logLine.message, privacy: .public)")
                        case .info: logger.info("\(logLine.message, privacy: .public)")
                        case .error: logger.critical("\(logLine.message, privacy: .public)")
                        case .unknown: logger.info("\(logLine.message, privacy: .public)")
                        }
                        publisher.yield(logLine)
                    }
            }
            publisher.onTermination = { _ in
                source.cancel()
                try? fileHandle.close()
            }
            source.resume()
        }
        self.logId = .init()
    }

    private func createVFSDirectory() {
        if FileManager.default.fileExists(atPath: Constants.libraryDir.path) == false {
            do {
                try FileManager.default.createDirectory(at: Constants.libraryDir, withIntermediateDirectories: true)
                try FileManager.default.copyItem(
                    at: Bundle.main.resourceURL!.appending(path: "PumpkinVFS"),
                    to: Constants.vfsUrl
                )
            } catch {
                Logger.default.critical("Failed to create VFS folders in library: \(error.log, privacy: .public)")
            }
        }
    }
}
