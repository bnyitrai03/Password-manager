# Password-manager

USB HID + Mass Storage Device

-Gombnyomásra vált a különböző USB funkciók között

STM32F407

Saját tömör rekordok flash-ben, a Mass Storage rész csak kommunikációs célból, vagy ténylegesen a flashbe van leképezve és fatfs olvassa az eszközben ugyanazt a területet
Ez egy egyszerű demo mass storage-hoz. https://controllerstech.com/stm32-usb-msc/

GUI: .NET vastag kliens alkalmazás

Milyen rekordokat érdemes tárolni? -Jelszó neve (pl "Facebook"), felhasználónév, jelszó (64+ bájt), mennyi TAB leütés a felhasználónév után, mennyi TAB leütés jelszó beírás után, mennyi ENTER leütés a jelszó után.
Mester jelszó engedi használni az eszközt, ezen alapul a titkosítás feloldása (enélkül nem olvashatók a tárolt rekordok). 
Titkosítás: chacha20 https://github.com/Ginurx/chacha20-c

Billentyűzet: rekordokban tárolt sztringek billentyű leütésekké konvertálása, ez függ az operációs rendszeren beállított nyelvtől, az eszköznek be kell tudni állítani, hogy milyen billentyűzet nyelv van beállítva a gépen. 
