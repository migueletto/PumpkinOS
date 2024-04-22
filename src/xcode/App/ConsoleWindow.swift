//
//  ConsoleWindow.swift
//
//  Created by peter bohac on 4/15/24.
//

import SwiftUI

struct ConsoleWindow: View {
    @AppStorage("globalLogLevel") private var globalLogLevel = Pit.LogLevel.info
    @AppStorage("moduleLogging") private var moduleLogging: [Pit.ModuleLogLevel] = []
    @AppStorage("width") private var width: Int = 650
    @AppStorage("height") private var height: Int = 492
    @State private var showModuleLoggingPopover = false
    @State private var showResetAlert = false
    @State private var logLines: [LogLine] = []
    @State private var filter = ""
    private var filteredLogLines: [LogLine] {
        guard filter.isEmpty == false else { return logLines }
        return logLines.filter { $0.message.contains(filter) }
    }

    @Environment(Pit.self) private var pit
    @Environment(\.openWindow) private var openWindow
    @Environment(\.dismissWindow) private var dismissWindow

    var body: some View {
        VStack {
            HStack {
                HStack {
                    Pit.LogLevel.picker("Logging level:", selection: $globalLogLevel)
                        .frame(maxWidth: 200)

                    Button("Module Logging") {
                        showModuleLoggingPopover.toggle()
                    }
                    .buttonStyle(.bordered)
                    .popover(isPresented: $showModuleLoggingPopover) {
                        ModuleLoggingPopover(items: $moduleLogging)
                            .frame(minWidth: 250, minHeight: 150)
                    }

                    LabeledContent("Width:") {
                        TextField("Width", value: $width, format: .number, prompt: Text("Width"))
                    }
                    .frame(maxWidth: 150)

                    LabeledContent("Height:") {
                        TextField("Height", value: $height, format: .number, prompt: Text("Height"))
                    }
                    .frame(maxWidth: 150)
                }
                .disabled(pit.running)

                Spacer()

                Button("Reset") {
                    showResetAlert.toggle()
                }
                .buttonStyle(.bordered)
                .disabled(pit.running)
                .alert("Please Confirm", isPresented: $showResetAlert) {
                    Button("Yes", role: .destructive) {
                        pit.reset()
                        logLines = []
                    }
                } message: {
                    Text("Are you sure you want to reset to a new state?")
                }

                Group {
                    if pit.running {
                        Button("Stop") {
                            Task {
                                await pit.stop()
                            }
                        }
                    } else {
                        Button("Start") {
                            logLines.removeAll()
                            pit.main(globalLogLevel: globalLogLevel, moduleLogging: moduleLogging, width: width, height: height)
                        }
                    }
                }
                .buttonStyle(.borderedProminent)
            }

            ScrollView {
                VStack {
                    ForEach(filteredLogLines) { line in
                        Text(line.message)
                            .foregroundStyle(line.level.style)
                    }
                    .fontDesign(.monospaced)
                    .frame(maxWidth: .infinity, alignment: .leading)
                }
            }
            .textSelection(.enabled)
            .focusable()
            .defaultScrollAnchor(.bottom)

            HStack {
                TextField("Log Filter", text: $filter, prompt: Text("Filter the logs"))
                    .textFieldStyle(.roundedBorder)
                Button("Clear Filter", systemImage: "xmark.circle") {
                    filter = ""
                }
                .labelStyle(.iconOnly)
                .buttonStyle(.plain)
            }
            .padding(.top, 2)
        }
        .padding(8)
        .navigationTitle("Pumpkin App Console")
        .task(id: pit.logId) {
            if let log = pit.logLines {
                for await line in log {
                    logLines.append(line)
                }
            }
        }
        .task {
            let events = await pit.windowProvider.events.subscribe()
            for await event in events {
                switch event {
                case .setTitle, .draw:
                    break
                case .createWindow(let window):
                    openWindow(id: "deviceWindow", value: window)
                case .destroyWindow(let window):
                    dismissWindow(id: "deviceWindow", value: window)
                }
            }
        }
    }
}

private struct ModuleLoggingPopover: View {
    @Binding var items: [Pit.ModuleLogLevel]

    var body: some View {
        VStack(alignment: .leading) {
            List {
                ForEach($items) { $item in
                    HStack {
                        Pit.LogLevel.picker("", selection: $item.level)
                        TextField("Module", text: $item.module, prompt: Text("Module name"))
                            .textFieldStyle(.squareBorder)
                        Button("Remove", systemImage: "trash", role: .destructive) {
                            items.removeAll(where: { $0.id == item.id })
                        }
                        .labelStyle(.iconOnly)
                        .buttonStyle(.plain)
                        .foregroundStyle(.red)
                    }
                }
            }
            .listStyle(.bordered)
            HStack {
                Button("Add", systemImage: "plus") {
                    items.append(.init(level: .debug, module: ""))
                }
                .labelStyle(.iconOnly)
                .buttonStyle(.bordered)
                Spacer()
                Button("Clear", role: .destructive) {
                    items.removeAll()
                }
                .buttonStyle(.plain)
                .foregroundStyle(.red)
            }
            .padding(.horizontal, 5)
            .padding(.bottom, 5)
        }
        .onAppear {
            if items.isEmpty {
                items.append(.init(level: .debug, module: ""))
            }
        }
        .onDisappear {
            items = items.compactMap { $0.module.trimmingCharacters(in: .whitespaces).isEmpty ? nil : $0 }
        }
    }
}

private extension Pit.LogLevel {
    static func picker(_ label: String, selection: Binding<Self>) -> some View {
        Picker(label, selection: selection) {
            ForEach(allCases, id: \.self) { level in
                Text(level.label).tag(level)
            }
        }
    }
    var label: String {
        switch self {
        case .debug: "Debug"
        case .info: "Info"
        case .error: "Error"
        }
    }
}

private extension LogLine.Level {
    var style: any ShapeStyle {
        switch self {
        case .trace: .secondary
        case .info: .primary
        case .error: .red
        case .unknown: .orange
        }
    }
}

extension Array: RawRepresentable where Element: Codable {
    public var rawValue: String {
        guard let data = try? JSONEncoder().encode(self) else { return "" }
        return String(data: data, encoding: .utf8) ?? ""
    }
    public init?(rawValue: String) {
        guard
            let data = rawValue.data(using: .utf8),
            let value = try? JSONDecoder().decode([Element].self, from: data)
        else { return nil }
        self = value
    }
}

#Preview {
    ConsoleWindow()
        .environment(Pit())
}
