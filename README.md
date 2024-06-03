# Password-manager
A cél STM32F407-es mikrokontrolleren kifejleszteni egy beágyazott szoftvert, amely Universal Serial Bus-on (USB) keresztül kommunikál egy asztali számítógéppel. Az eszköz gombnyomásra képes a rajta eltárolt jelszavakat billentyűleütésekként beírni a számítógépen megnyitott weboldalra vagy alkalmazásra. A tárolás perzisztens módon a mikrokontroller flash memóriájában legyen megvalósítva tömör rekordokként. Egy rekord tartalmazza az alkalmazás nevét, ahova szeretnénk bejelentkezni, az ehhez tartozó felhasználónevet és jelszót és ezek beírásához szükséges tabokat, entereket. A rekordok chacha20 algoritmussal legyenek titkosítva az eszköz feloldásáig. A felhasználó tudja módosítani a jelszavakat, illetve vehessen fel újakat, és törölhesse is őket. Az eszközön lehessen beállítani, hogy milyen nyelvű billentyűzet van a számítógéphez csatlakoztatva, hogy ennek megfelelő billentyűleütések kerüljenek elküldésre.

## Működés
Az eszköz működésének az alapját egy USB Composite Device adja. Ennek az eszközosztálynak a használata meglehetősen előnyős a probléma megoldására. Alkalmazásával lehetséges több különböző USB funkciót ellátni egyetlen fizikai eszközön belül, így lehetőség nyílik a teljesen különböző feladatokat végző komponensek működésének egyszerű összehangolására. A Composite Device egyszerre valósít meg egy Human Interface Device (HID) osztályt, egy Mass Storage Class-t (MSC) és egy Communication Device Class-t (CDC) is. 

  Az USB HID implementálja a billentyűzetet, ennek az eszközosztálynak a felelőssége a billentyűkódok továbbítása a személyi számítógép felé. Az USB CDC teremt kapcsolatot az eszköz és a felhasználó között. A felhasználó egy soros portot kezelő alkalmazáson (PuTTY, hercules) keresztül tud vezérlő üzeneteket küldeni a mikrokontrollernek, amely ezeknek az utasításoknak megfelelő műveleteket hajt végre. A vezérlő üzenetek pontos működése később lesz részletezve. A harmadik eszközosztály az USB MSC pedig az adatok Random Access Memory-ban (RAM) történő tárolását, és ezek módosítását, törlését valósítja meg. Ez az osztály teszi lehetővé, hogy az eszköz a host számítógépről egy külső meghajtónak tűnjön, így egyszerűen fájlokként lehet kezelni a különböző jelszavakat tároló rekordokat.
  
A fájlok tárolását és kezelését segíti a RAM-ba leképezett File Allocation Table file system (Fatfs). Használata lehetővé teszi az új jelszavak könnyű eltárolását és a már meglévő rekordok listázását. A Fatfs-ben tárolt fájlok szinkronizálva vannak az USB MSC-vel és a mikrokontroller belső flash memóriájával is, tehát ha az asztali számítógépen elmentünk egy fájlt a külső háttértárolóra, az a fájl a Fatfs-be bekerüléskor automatikusan mentődik a belső flash memóriába is, ahol a tápfeszültség megszűnése után is megörződik.


## Flowchart
![image](https://github.com/bnyitrai03/Password-manager/assets/126956031/639d4c2b-4a91-4721-b7d8-f4e96be1a4fa)

## UI
![image](https://github.com/bnyitrai03/Password-manager/assets/126956031/982af785-309f-4c86-ad11-c8fbaed349d5)
