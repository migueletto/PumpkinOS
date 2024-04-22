//
//  Logging.swift
//
//  Created by peter bohac on 4/15/24.
//

import OSLog

extension Logger {
    static var `default`: Self { .init(subsystem: Bundle.main.bundleIdentifier!, category: "default") }
    static var pit: Self { .init(subsystem: Bundle.main.bundleIdentifier!, category: "pit") }
    static var wp: Self { .init(subsystem: Bundle.main.bundleIdentifier!, category: "windowProvider") }
}

extension Error {
    var log: String { String(describing: self) }
}

struct LogLine: Identifiable, Sendable {
    enum Level {
        case trace, info, error, unknown
    }
    let id = UUID()
    let message: String
    let level: Level

    init(_ line: String) {
        self.message = String(line)
        let parts = line.split(separator: " ")
        if parts.count >= 3 {
            switch parts[2] {
            case "T": self.level = .trace
            case "I": self.level = .info
            case "E": self.level = .error
            default: self.level = .unknown
            }
        } else {
            self.level = .unknown
        }
    }
}
