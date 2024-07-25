import 'package:flutter_test/flutter_test.dart';
import 'package:mobile_scanner/src/enums/encryption_type.dart';

void main() {
  group('$EncryptionType tests', () {
    test('can be created from raw value', () {
      const values = <int, EncryptionType>{
        0: EncryptionType.none,
        1: EncryptionType.open,
        2: EncryptionType.wpa,
        3: EncryptionType.wep,
      };

      for (final MapEntry<int, EncryptionType> entry in values.entries) {
        final EncryptionType result = EncryptionType.fromRawValue(entry.key);

        expect(result, entry.value);
      }
    });

    test('invalid raw value throws argument error', () {
      const int negative = -1;
      const int outOfRange = 4;

      expect(() => EncryptionType.fromRawValue(negative), throwsArgumentError);
      expect(
        () => EncryptionType.fromRawValue(outOfRange),
        throwsArgumentError,
      );
    });

    test('can be converted to raw value', () {
      const values = <EncryptionType, int>{
        EncryptionType.none: 0,
        EncryptionType.open: 1,
        EncryptionType.wpa: 2,
        EncryptionType.wep: 3,
      };

      for (final MapEntry<EncryptionType, int> entry in values.entries) {
        final int result = entry.key.rawValue;

        expect(result, entry.value);
      }
    });
  });
}
