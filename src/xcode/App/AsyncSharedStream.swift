//
//  AsyncSharedStream.swift
//
//  Created by peter bohac on 4/16/24.
//

import Foundation

public actor AsyncSharedStream<Element: Sendable>: Sendable {
    enum State {
        case started
        case completed
    }

    private let source: AsyncStream<Element>
    private var subscribers: [UUID: AsyncStream<Element>.Continuation]
    private var state: State

    public init(_ source: AsyncStream<Element>) {
        self.source = source
        self.subscribers = [:]
        self.state = .started
        Task {
            for await element in source {
                await subscribers.values.forEach { $0.yield(element) }
            }
            await complete()
        }
    }

    public func subscribe() -> AsyncStream<Element> {
        let (stream, continuation) = AsyncStream<Element>.makeStream()
        switch state {
        case .started:
            let id = UUID()
            subscribers[id] = continuation
            continuation.onTermination = { [weak self] _ in
                guard let self else { return }
                Task {
                    await self.removeSubscriber(id: id)
                }
            }
        case .completed:
            continuation.finish()
        }
        return stream
    }

    private func complete() {
        subscribers.values.forEach { $0.finish() }
        state = .completed
    }

    private func removeSubscriber(id: UUID) {
        subscribers[id] = nil
    }
}
