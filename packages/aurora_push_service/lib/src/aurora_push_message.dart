// Copyright (c) 2023, Friflex LLC. Please see the AUTHORS file
// for details. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

/// Модель с данными о пуш уведомлении.
///
/// Подробнее:
/// * https://developer.auroraos.ru/doc/4.0.2/software_development/guides/push/client
class AuroraPushMessage {
  /// Заголовок сообщения.
  final String? title;

  /// Тело сообщения.
  final String? message;

  /// Строка, содержащая сериализованный JSON-объект.
  final String? data;

  /// Зарезервировано на будущее.
  final String? action;

  /// Зарезервировано на будущее.
  final String? sound;

  /// Зарезервировано на будущее.
  final String? image;

  /// Зарезервировано на будущее.
  final int? categoryId;

  /// Модель с данными о пуш уведомлении.
  const AuroraPushMessage({
    this.title,
    this.message,
    this.data,
    this.action,
    this.sound,
    this.image,
    this.categoryId,
  });

  /// Returns the [AuroraPushMessage] from a raw [Map].
  factory AuroraPushMessage.fromMap(Map<String, dynamic> map) {
    return AuroraPushMessage(
      title: map['title'],
      data: map['data'],
      action: map['action'],
      message: map['message'],
      sound: map['sound'],
      image: map['image'],
      categoryId: map['categoryId'],
    );
  }

  /// Returns the [AuroraPushMessage] as a raw [Map].
  Map<String, dynamic> toMap() {
    return <String, dynamic>{
      'title': title,
      'data': data,
      'action': action,
      'message': message,
      'sound': sound,
      'image': image,
      'categoryId': categoryId,
    };
  }
}
