# Setup fuer Entwicklung und Paketierung

## Ziel

Diese Datei beschreibt, wie ein frisches Linux-System so vorbereitet wird, dass mit diesem Repository sowohl

- die normale Entwicklung
- als auch die Debian-Paketierung

moeglich ist.

Sie ist auf den aktuellen Stand von CoverFlowMP zugeschnitten.

## 1. Voraussetzungen

## 1.1 Zielsystem

Diese Anleitung ist fuer Debian- oder Ubuntu-basierte Systeme gedacht, weil das Projekt mit Debian-Werkzeugen paketiert wird und die genannten Pakete per `apt` installiert werden.

Empfohlen:

- Debian
- Ubuntu
- Linux Mint oder ein anderes apt-basiertes Derivat

## 1.2 Was das System koennen muss

Das System muss mindestens Folgendes bereitstellen:

- `apt`
- einen C++17-faehigen Compiler
- Qt 6 Entwicklungsdateien
- libmpv Entwicklungsdateien
- Debian-Paketierungswerkzeuge

## 1.3 Wofuer der Rechner danach vorbereitet ist

Nach dieser Einrichtung soll das System:

- das Projekt mit CMake bauen koennen
- das Programm lokal starten koennen
- Debian-Pakete mit `debuild` und `dpkg-buildpackage` erzeugen koennen

## 2. Repository holen

Auf einem frischen System wird zuerst das Repository benoetigt.

Beispiel:

```bash
git clone <REPOSITORY-URL> CoverFlowMP
cd CoverFlowMP
```

Wenn das Repository bereits lokal vorliegt, reicht:

```bash
cd /pfad/zu/CoverFlowMP
```

## 3. Systempakete fuer Entwicklung und Packaging installieren

## 3.1 Paketlisten aktualisieren

```bash
sudo apt update
```

## 3.2 Basis-Werkzeuge fuer Entwicklung

Diese Pakete werden fuer das Bauen des Projekts gebraucht:

```bash
sudo apt install \
  build-essential \
  cmake \
  pkg-config \
  git
```

Bedeutung:

- `build-essential`: Compiler, Linker und Basis-Buildtools
- `cmake`: Buildsystem des Projekts
- `pkg-config`: findet unter anderem `libmpv`
- `git`: fuer den Checkout und die Pflege des Repositories

## 3.3 Qt- und mpv-Entwicklungsabhaengigkeiten

Diese Pakete sind fuer CoverFlowMP als Projekt direkt erforderlich:

```bash
sudo apt install \
  qt6-base-dev \
  qt6-declarative-dev \
  qt6-tools-dev \
  libmpv-dev
```

Warum diese Pakete noetig sind:

- `qt6-base-dev`: Qt Core, Gui und DBus Entwicklungsdateien
- `qt6-declarative-dev`: Qt Quick und QML Entwicklung
- `qt6-tools-dev`: Qt Build-Werkzeuge
- `libmpv-dev`: Header und Linker-Dateien fuer die mpv-Integration

## 3.4 Debian-Werkzeuge fuer das Packaging

Diese Pakete werden benoetigt, damit die Debian-Befehle aus diesem Projekt funktionieren:

```bash
sudo apt install \
  devscripts \
  dpkg-dev \
  debhelper \
  fakeroot
```

Bedeutung:

- `devscripts`: liefert unter anderem `debuild` und `dch`
- `dpkg-dev`: liefert unter anderem `dpkg-buildpackage`
- `debhelper`: liefert `dh`
- `fakeroot`: wird haeufig beim Debian-Build genutzt

## 3.5 Laufzeitpakete fuer lokale Programmstarts

Damit das Programm nach einem lokalen Build oder nach einer lokalen Paketinstallation direkt laeuft, sind diese QML- und SQLite-Runtime-Pakete sinnvoll:

```bash
sudo apt install \
  libqt6sql6-sqlite \
  qml6-module-qt5compat-graphicaleffects \
  qml6-module-qtqml \
  qml6-module-qtqml-workerscript \
  qml6-module-qtquick \
  qml6-module-qtquick-controls \
  qml6-module-qtquick-layouts \
  qml6-module-qtquick-templates \
  qml6-module-qtquick-window
```

Diese Pakete sind fuer das lokale Testen besonders wichtig, auch wenn ein reiner Compiler-Setup manchmal schon frueher erfolgreich konfiguriert.

## 3.6 Alles in einem Schritt installieren

Wenn das System in einem Durchgang vorbereitet werden soll, kann diese Kombination verwendet werden:

```bash
sudo apt update
sudo apt install \
  build-essential \
  cmake \
  pkg-config \
  git \
  devscripts \
  dpkg-dev \
  debhelper \
  fakeroot \
  qt6-base-dev \
  qt6-declarative-dev \
  qt6-tools-dev \
  libmpv-dev \
  libqt6sql6-sqlite \
  qml6-module-qt5compat-graphicaleffects \
  qml6-module-qtqml \
  qml6-module-qtqml-workerscript \
  qml6-module-qtquick \
  qml6-module-qtquick-controls \
  qml6-module-qtquick-layouts \
  qml6-module-qtquick-templates \
  qml6-module-qtquick-window
```

## 4. Projektstruktur pruefen

Nach dem Klonen sollte das Projekt mindestens diese wichtigen Elemente enthalten:

- `CMakeLists.txt`
- `main.cpp`
- `src/`
- `qml/`
- `assets/`
- `resources.qrc`
- `packaging/`
- `debian/`
- `README.md`
- `packaging.md`

Wenn `debian/` oder `CMakeLists.txt` fehlen, ist das Repository nicht vollstaendig fuer die Paketierung.

## 5. Entwicklungs-Build aufsetzen

## 5.1 Build-Ordner anlegen

Beispiel mit einem klassischen lokalen Build-Ordner:

```bash
mkdir -p build
cmake -S . -B build
```

Alternativ kannst du auch ein anderes Build-Verzeichnis verwenden, zum Beispiel:

```bash
cmake -S . -B obj-local
```

## 5.2 Projekt bauen

```bash
cmake --build build
```

Oder mit dem alternativen Verzeichnis:

```bash
cmake --build obj-local
```

## 5.3 Programm lokal starten

Nach einem erfolgreichen Build liegt das Binary im Build-Verzeichnis.

Beispiel:

```bash
./build/CoverFlowMPApp
```

Optional mit Root-Video-Ordner:

```bash
./build/CoverFlowMPApp /pfad/zum/video-root
```

Wenn kein Pfad angegeben wird, verwendet CoverFlowMP standardmaessig `~/Videos`.

## 6. Entwicklungsumgebung verifizieren

Wenn du pruefen willst, ob die wichtigsten Werkzeuge vorhanden sind, helfen diese Befehle:

```bash
cmake --version
pkg-config --modversion Qt6Core
pkg-config --modversion Qt6DBus
pkg-config --modversion mpv
debuild --version
dpkg-buildpackage --version
dch --version
```

Wenn diese Befehle ohne Fehler laufen, ist der Setup in der Regel weitgehend vollstaendig.

## 7. Debian-Paketierungsumgebung verifizieren

## 7.1 Debian-Metadaten pruefen

Diese Dateien muessen vorhanden sein:

- `debian/control`
- `debian/changelog`
- `debian/rules`
- `debian/source/format`

Optional, aber fuer das Projekt sinnvoll:

- `debian/copyright`
- weitere gepflegte Debian-Hilfsdateien

## 7.2 Build-Abhaengigkeiten gegen das System pruefen

Dieses Projekt erwartet aktuell in `debian/control` unter `Build-Depends`:

- `debhelper-compat (= 13)`
- `cmake`
- `pkg-config`
- `qt6-base-dev`
- `qt6-declarative-dev`
- `qt6-tools-dev`
- `libmpv-dev`

Wenn diese Pakete installiert sind, ist die Debian-Build-Seite fuer dieses Projekt im Kern vorbereitet.

## 8. Debian-Paket bauen

## 8.1 Empfohlene Aufraeumfolge

Vor einem neuen Paketbau kannst du diesen Ablauf verwenden:

```bash
debuild -- clean
rm -rf build obj-* debian/coverflowmp
```

Hinweis:

- Das projektbezogene Debian-Staging-Verzeichnis ist `debian/coverflowmp`.
- Dieser Pfad passt zum aktuellen Paketnamen.

## 8.2 Debian-Version anheben

Vor dem Paketbau wird ueblicherweise die Version gepflegt:

```bash
dch -v x.y.z "Version info"
```

Beispiel:

```bash
dch -v 1.2.5 "Update package for latest player changes"
```

## 8.3 Paket bauen

```bash
dpkg-buildpackage -us -uc -b
```

Bedeutung:

- `-us`: Source-Paket nicht signieren
- `-uc`: `.changes` nicht signieren
- `-b`: nur Binärpakete bauen

## 8.4 Ergebnis pruefen

Die erzeugten Dateien liegen typischerweise eine Ebene oberhalb des Projektordners.

Beispiele:

- `../coverflowmp_<version>_<arch>.deb`
- `../coverflowmp-dbgsym_<version>_<arch>.deb`
- `../coverflowmp_<version>_<arch>.buildinfo`
- `../coverflowmp_<version>_<arch>.changes`

## 9. Paket lokal testen

Nach dem Paketbau kannst du das erstellte Debian-Paket lokal installieren:

```bash
sudo dpkg -i ../coverflowmp_<version>_<arch>.deb
```

Wenn dabei noch Laufzeitabhaengigkeiten fehlen sollten, kann das oft mit folgendem Befehl aufgeloest werden:

```bash
sudo apt -f install
```

Danach kann die Anwendung normal gestartet werden.

Je nach Installation und Desktop-Datei zum Beispiel ueber:

```bash
coverflowmp
```

oder direkt ueber das installierte Binary.

## 10. Typische Probleme auf frischen Systemen

## 10.1 `debuild` oder `dch` fehlt

Dann ist meist `devscripts` nicht installiert.

Loesung:

```bash
sudo apt install devscripts
```

## 10.2 `dpkg-buildpackage` fehlt

Dann fehlt meist `dpkg-dev`.

Loesung:

```bash
sudo apt install dpkg-dev
```

## 10.3 `dh` fehlt

Dann fehlt meist `debhelper`.

Loesung:

```bash
sudo apt install debhelper
```

## 10.4 Qt oder QML wird beim Build nicht gefunden

Dann fehlen meist eine oder mehrere dieser Pakete:

- `qt6-base-dev`
- `qt6-declarative-dev`
- `qt6-tools-dev`

Loesung:

```bash
sudo apt install qt6-base-dev qt6-declarative-dev qt6-tools-dev
```

## 10.5 `mpv` wird beim Build nicht gefunden

Dann fehlt meist `libmpv-dev`.

Loesung:

```bash
sudo apt install libmpv-dev
```

## 10.6 Das Binary startet lokal nicht wegen QML-Modulen

Dann fehlen meist Runtime-Pakete fuer Qt Quick / QML.

Loesung:

Installiere die in Abschnitt 3.5 genannten Laufzeitpakete.

## 11. Kurzfassung fuer ein frisches System

Wenn du moeglichst direkt loslegen willst, reicht fuer ein frisches Debian-/Ubuntu-System in der Regel diese Folge:

```bash
sudo apt update
sudo apt install \
  build-essential \
  cmake \
  pkg-config \
  git \
  devscripts \
  dpkg-dev \
  debhelper \
  fakeroot \
  qt6-base-dev \
  qt6-declarative-dev \
  qt6-tools-dev \
  libmpv-dev \
  libqt6sql6-sqlite \
  qml6-module-qt5compat-graphicaleffects \
  qml6-module-qtqml \
  qml6-module-qtqml-workerscript \
  qml6-module-qtquick \
  qml6-module-qtquick-controls \
  qml6-module-qtquick-layouts \
  qml6-module-qtquick-templates \
  qml6-module-qtquick-window

git clone <REPOSITORY-URL> CoverFlowMP
cd CoverFlowMP

cmake -S . -B build
cmake --build build

debuild -- clean
rm -rf build obj-* debian/coverflowmp
dch -v x.y.z "Version info"
dpkg-buildpackage -us -uc -b
```

## 12. Fazit

Wenn die in dieser Datei genannten Pakete installiert sind und das Repository vollstaendig vorliegt, ist ein frisches System fuer CoverFlowMP so vorbereitet, dass:

- Entwicklung moeglich ist
- lokale Programmstarts moeglich sind
- Debian-Paketierung moeglich ist

Diese Datei ist damit die konkrete Einrichtungsanleitung, waehrend [packaging.md](/home/devnull69/Projects/CoverFlowMP/packaging.md) die Paketierungslogik und die Projektstruktur beschreibt.
