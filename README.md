# Password-manager

UCB Composite Device: USB HID + Mass Storage Device + USB ACM

STM32F407

Saját tömör rekordok Flash-ben, a Mass Storage rész kommunikációs célból és a RAM-ban levő Fatfs olvassa az eszközben ugyanazt a területet
Ez egy egyszerű demo mass storage-hoz. https://controllerstech.com/stm32-usb-msc/

GUI: konzolos alkalmazás

Milyen rekordokat érdemes tárolni? -Jelszó neve (pl "Facebook"), felhasználónév, jelszó (64+ bájt), mennyi TAB leütés a felhasználónév után, mennyi TAB leütés jelszó beírás után, mennyi ENTER leütés a jelszó után.
Mester jelszó engedi használni az eszközt, ezen alapul a titkosítás feloldása (enélkül nem olvashatók a tárolt rekordok). 
Titkosítás: chacha20 https://github.com/Ginurx/chacha20-c

Billentyűzet: rekordokban tárolt sztringek billentyű leütésekké konvertálása, ez függ az operációs rendszeren beállított nyelvtől, az eszköznek be kell tudni állítani, hogy milyen billentyűzet nyelv van beállítva a gépen. 

-Gombnyomásra beírja a jelszót a weboldalra

## VCP kommunikáció
![PassTool_VCP_parancsok](https://github.com/bnyitrai03/Password-manager/assets/126956031/6d411d40-8312-4c79-b56b-1ed600136ada)
