0) Подключаете телефон в режиме разработчика
1) Собираете под аврору
flutter build aurora --release

2) Копируете полную ссылку на файл
если что она находится по пути /build/aurora/arm/release/RPMS/
пример:
/home/mozerrr/src/projects/aurora_test_lab/packages/aurora_push_service/example/build/aurora/arm/release/RPMS/com.example.aurora_push_example-0.1.0-1.armv7hl.rpm

3) Делаете подпись (сейчас только тестовый серт и ключ)
aurora_psdk rpmsign-external sign --key /home/mozerrr/src/aurora/cert/regular_key.pem --cert /home/mozerrr/src/aurora/cert/regular_cert.pem /home/mozerrr/src/projects/aurora_test_lab/packages/aurora_push_service/example/build/aurora/arm/release/RPMS/com.example.aurora_push_example-0.1.0-1.armv7hl.rpm

4) Копируете на телефон
scp /home/mozerrr/src/projects/aurora_test_lab/packages/aurora_push_service/example/build/aurora/arm/release/RPMS/com.example.aurora_push_example-0.1.0-1.armv7hl.rpm defaultuser@192.168.2.15:/home/defaultuser/Downloads

5) Устанавливаете
ssh defaultuser@192.168.2.15
devel-su
pkcon install-local /home/defaultuser/Downloads/com.example.aurora_push_example-0.1.0-1.armv7hl.rpm -y