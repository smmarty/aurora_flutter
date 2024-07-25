//
//  BarcodeHandler.swift
//  mobile_scanner
//
//  Created by Julian Steenbakker on 24/08/2022.
//

import Flutter
import Foundation

public class BarcodeHandler: NSObject, FlutterStreamHandler {
    private var eventSink: FlutterEventSink?
    private let eventChannel: FlutterEventChannel
    
    init(registrar: FlutterPluginRegistrar) {
        eventChannel = FlutterEventChannel(name:
                                            "dev.steenbakker.mobile_scanner/scanner/event", binaryMessenger: registrar.messenger())
        super.init()
        eventChannel.setStreamHandler(self)
    }
    
    func publishEvent(_ event: [String: Any?]) {
        DispatchQueue.main.async {
            self.eventSink?(event)
        }
    }
    
    public func onListen(withArguments arguments: Any?,
                         eventSink: @escaping FlutterEventSink) -> FlutterError? {
        self.eventSink = eventSink
        return nil
    }
    
    public func onCancel(withArguments arguments: Any?) -> FlutterError? {
        eventSink = nil
        return nil
    }
}
