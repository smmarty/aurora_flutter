// Copyright (c) 2024, Friflex LLC. Please see the AUTHORS file
// for details. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';
import 'package:video_player_platform_interface/video_player_platform_interface.dart';

const MethodChannel _channel = MethodChannel('friflex/video_player_aurora');

class VideoPlayerAurora extends VideoPlayerPlatform {
  static void registerWith() {
    VideoPlayerPlatform.instance = VideoPlayerAurora();
  }

  @override
  Future<void> init() {
    return _channel.invokeMethod('initialize');
  }

  @override
  Future<void> dispose(int textureId) {
    final message = {'textureId': textureId};

    return _channel.invokeMethod('dispose', message);
  }

  @override
  Future<int?> create(DataSource dataSource) async {
    String? asset;
    String? packageName;
    String? uri;
    String? formatHint;
    Map<String, String> httpHeaders = {};
    switch (dataSource.sourceType) {
      case DataSourceType.asset:
        asset = dataSource.asset;
        packageName = dataSource.package;
        break;
      case DataSourceType.network:
        uri = dataSource.uri;
        formatHint = _videoFormatStringMap[dataSource.formatHint];
        httpHeaders = dataSource.httpHeaders;
        break;
      case DataSourceType.file:
        uri = dataSource.uri;
        httpHeaders = dataSource.httpHeaders;
        break;
      case DataSourceType.contentUri:
        uri = dataSource.uri;
        break;
    }

    final message = <String, Object?>{};

    message['asset'] = asset;
    message['uri'] = uri;
    message['packageName'] = packageName;
    message['formatHint'] = formatHint;
    message['httpHeaders'] = httpHeaders;

    final result = await _channel.invokeMethod('create', message);
    if (result == null) return -1;
    final Map<Object?, Object?> map = result["result"] as Map<Object?, Object?>;
    return int.tryParse(map['textureId'].toString()) ?? -1;
  }

  @override
  Future<void> setLooping(int textureId, bool looping) {
    final message = {'textureId': textureId, 'isLooping': looping};

    return _channel.invokeMethod('setLooping', message);
  }

  @override
  Future<void> play(int textureId) {
    final message = {'textureId': textureId};

    return _channel.invokeMethod('play', message);
  }

  @override
  Future<void> pause(int textureId) {
    final message = {'textureId': textureId};

    return _channel.invokeMethod('pause', message);
  }

  @override
  Future<void> setVolume(int textureId, double volume) {
    final message = {'textureId': textureId, 'volume': volume};

    return _channel.invokeMethod('setVolume', message);
  }

  @override
  Future<void> setPlaybackSpeed(int textureId, double speed) {
    final message = {'textureId': textureId, 'speed': speed};

    return _channel.invokeMethod('setPlaybackSpeed', message);
  }

  @override
  Future<void> seekTo(int textureId, Duration position) {
    final message = {
      'textureId': textureId,
      'position': position.inMilliseconds,
    };

    return _channel.invokeMethod('seekTo', message);
  }

  @override
  Future<Duration> getPosition(int textureId) async {
    final message = {'textureId': textureId};

    final result = await _channel.invokeMethod('getPosition', message);
    return Duration(milliseconds: result?['position'] as int? ?? 0);
  }

  @override
  Stream<VideoEvent> videoEventsFor(int textureId) {
    return _eventChannelFor(textureId)
        .receiveBroadcastStream()
        .map((dynamic event) {
      final map = event as Map<dynamic, dynamic>;
      switch (map['event']) {
        case 'initialized':
          return VideoEvent(
            eventType: VideoEventType.initialized,
            duration: Duration(milliseconds: map['duration'] as int),
            size: Size((map['width'] as num?)?.toDouble() ?? 0.0,
                (map['height'] as num?)?.toDouble() ?? 0.0),
            rotationCorrection: map['rotationCorrection'] as int? ?? 0,
          );
        case 'completed':
          return VideoEvent(
            eventType: VideoEventType.completed,
          );
        case 'bufferingUpdate':
          final List<dynamic> values = map['values'] as List<dynamic>;

          return VideoEvent(
            buffered: values.map<DurationRange>(_toDurationRange).toList(),
            eventType: VideoEventType.bufferingUpdate,
          );
        case 'bufferingStart':
          return VideoEvent(eventType: VideoEventType.bufferingStart);
        case 'bufferingEnd':
          return VideoEvent(eventType: VideoEventType.bufferingEnd);
        case 'isPlayingStateUpdate':
          return VideoEvent(
            eventType: VideoEventType.isPlayingStateUpdate,
            isPlaying: map['isPlaying'] as bool,
          );
        default:
          return VideoEvent(eventType: VideoEventType.unknown);
      }
    });
  }

  @override
  Widget buildView(int textureId) {
    return Texture(textureId: textureId);
  }

  @override
  Future<void> setMixWithOthers(bool mixWithOthers) {
    final message = {mixWithOthers: mixWithOthers};

    return _channel.invokeMethod('setMixWithOthers', message);
  }

  EventChannel _eventChannelFor(int textureId) {
    return EventChannel(
      'friflex/video_player_aurora/events$textureId',
    );
  }

  static const Map<VideoFormat, String> _videoFormatStringMap =
      <VideoFormat, String>{
    VideoFormat.ss: 'ss',
    VideoFormat.hls: 'hls',
    VideoFormat.dash: 'dash',
    VideoFormat.other: 'other',
  };

  DurationRange _toDurationRange(dynamic value) {
    final List<dynamic> pair = value as List<dynamic>;
    return DurationRange(
      Duration(milliseconds: pair[0] as int),
      Duration(milliseconds: pair[1] as int),
    );
  }
}
