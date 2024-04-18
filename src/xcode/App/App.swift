//
//  App.swift
//
//  Created by peter bohac on 4/15/24.
//

import OSLog
import SwiftUI

@main
struct PumpkinApp: App {
    @NSApplicationDelegateAdaptor(AppDelegate.self) private var appDelegate
    private let pit = Pit()

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environment(pit)
        }

        WindowGroup(id: "deviceWindow", for: WindowProvider.Window.self) { window in
            DeviceWindow(window: window.wrappedValue ?? .empty)
                .environment(pit)
        }
        .windowResizability(.contentSize)
    }
}

class AppDelegate: NSObject, NSApplicationDelegate {
    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        return true
    }

    func applicationWillFinishLaunching(_ notification: Notification) {
        flushSavedWindowState()
    }

    func flushSavedWindowState() {
        do {
            let appPersistentStateDir = Bundle.main.bundleIdentifier!.appending(".savedState")
            let windowsPlistFile = URL.libraryDirectory
                .appending(path: "Saved Application State")
                .appending(path: appPersistentStateDir)
                .appending(path: "windows.plist")
            try FileManager.default.removeItem(at: windowsPlistFile)
        } catch {
            Logger.default.warning("Unable to delete window state file: \(String(describing: error), privacy: .public)")
        }
    }
}
