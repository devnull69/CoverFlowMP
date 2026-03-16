# Debian Packaging fuer CoverFlowMP

## Ziel

Diese Datei beschreibt, wie dieses Projekt aufgebaut sein muss, damit es mit genau dieser Schrittfolge als Debian-Paket gebaut werden kann:

```bash
debuild -- clean
rm -rf build obj-* debian/coverflowmp
dch -v x.y.z "Version info"
dpkg-buildpackage -us -uc -b
```

Zusatz fuer dieses konkrete Projekt:

- Der Debian-Quellpaketname ist `coverflowmp`.
- Das Binärpaket heisst ebenfalls `coverflowmp`.
- Die Quellformat-Datei ist bereits auf `3.0 (native)` gesetzt.

Die Datei erklaert deshalb sowohl den erwarteten Projektaufbau als auch die Bedeutung jedes Schritts.

## 1. Erforderlicher Projektaufbau

Damit die oben genannte Build-Sequenz funktioniert, muss das Projekt in dieser Form vorliegen.

### 1.1 Projektwurzel

Die Befehle muessen im Hauptordner des Projekts ausgefuehrt werden. Dort muessen mindestens diese Dinge vorhanden sein:

- `CMakeLists.txt`
- `main.cpp`
- `src/`
- `qml/`
- `assets/`
- `resources.qrc`
- `packaging/`
- `debian/`

Fuer dieses Projekt ist das bereits der Fall.

### 1.2 CMake-Projekt muss installierbar sein

Der Debian-Build mit `dh` und `cmake` funktioniert nur sauber, wenn das Projekt nicht nur gebaut, sondern auch per `cmake --install` installiert werden kann.

Das ist hier wichtig, weil `debian/rules` aktuell einfach dieses Debhelper-Schema verwendet:

```make
#!/usr/bin/make -f
%:
	dh $@ --buildsystem=cmake
```

Das bedeutet:

- `dh_auto_configure` konfiguriert CMake
- `dh_auto_build` baut das Projekt
- `dh_auto_install` fuehrt die CMake-Install-Regeln aus
- Debhelper packt danach die installierten Dateien in das `.deb`

Deshalb muessen in `CMakeLists.txt` funktionierende `install(...)`-Regeln vorhanden sein.

Fuer dieses Projekt sind aktuell insbesondere diese Installationspunkte wichtig:

- das Binary `CoverFlowMPApp`
- die Desktop-Datei `packaging/coverflowmp.desktop`
- das App-Icon `assets/appicon.png`

Wenn spaeter weitere Dateien systemweit installiert werden sollen, muessen sie ebenfalls ueber `install(...)` eingebunden werden.

### 1.3 Debian-Verzeichnis muss vorhanden und konsistent sein

Der Ordner `debian/` ist fuer den Paketbau zwingend. Fuer dieses Projekt sind vor allem diese Dateien relevant:

- `debian/control`
- `debian/changelog`
- `debian/rules`
- `debian/copyright`
- `debian/source/format`

Die Rollen sind:

- `debian/control`: definiert Quellpaket, Binärpaket und Build-Abhaengigkeiten
- `debian/changelog`: definiert die Debian-Paketversion
- `debian/rules`: startet den Debhelper-/CMake-Build
- `debian/source/format`: legt das Paketformat fest
- `debian/copyright`: wird fuer ein sauberes Debian-Paket erwartet

Fuer dieses Projekt ist besonders wichtig:

- In `debian/source/format` steht `3.0 (native)`
- In `debian/control` steht als Source-Paket `coverflowmp`
- In `debian/control` steht als Binary-Paket ebenfalls `coverflowmp`

### 1.4 Native-Paket statt Upstream-Tarball

Dieses Projekt ist aktuell als natives Debian-Paket konfiguriert:

```text
3.0 (native)
```

Das hat direkte Folgen:

- Es wird kein separates `orig.tar.*` benoetigt
- Die Debian-Version steht direkt in `debian/changelog`
- `dpkg-buildpackage -b` kann direkt aus dem Arbeitsbaum heraus verwendet werden

Wenn das Projekt spaeter in ein nicht-natives Paket umgestellt wird, aendert sich der Ablauf deutlich.

### 1.5 Paketname, Versionspflege und Namenskonstanz

Damit der Ablauf stabil bleibt, muessen diese Namen konsistent bleiben:

- Quellpaketname in `debian/control`: `coverflowmp`
- Binärpaketname in `debian/control`: `coverflowmp`
- Eintrag in `debian/changelog`: `coverflowmp (VERSION) ...`

Wenn der Paketname geaendert wird, muessen diese Stellen gleichzeitig angepasst werden:

- `debian/control`
- `debian/changelog`
- eventuelle generierte Staging-Verzeichnisse unter `debian/`
- alle Dokumentationen und Build-Skripte

### 1.6 Kein `debian/install` noetig, solange CMake alles installiert

In diesem Projekt existiert derzeit keine `debian/install`.
Das ist in Ordnung, solange alle zu paketierenden Dateien ueber `CMakeLists.txt` installiert werden.

Falls spaeter Dateien nicht per CMake installiert werden, gibt es zwei saubere Wege:

1. Die fehlenden Dateien per `install(...)` in `CMakeLists.txt` ergaenzen.
2. Alternativ gezielt eine `debian/install` anlegen.

Fuer den aktuellen Stand ist der erste Weg bereits umgesetzt und ausreichend.

## 2. Was auf dem Linux-System installiert sein muss

Damit genau dieser Ablauf fuer dieses Projekt funktioniert, braucht das System sowohl die Debian-Paketierungswerkzeuge als auch die projektspezifischen Build-Abhaengigkeiten.

## 2.1 Allgemeine Debian-Paketierungswerkzeuge

Mindestens diese Pakete sollten installiert sein:

```bash
sudo apt install build-essential devscripts dpkg-dev debhelper fakeroot
```

Warum diese Pakete wichtig sind:

- `build-essential`: Compiler, Linker und Basis-Buildtools
- `devscripts`: liefert unter anderem `debuild` und `dch`
- `dpkg-dev`: liefert unter anderem `dpkg-buildpackage`
- `debhelper`: liefert `dh` und die Debhelper-Infrastruktur
- `fakeroot`: wird haeufig beim Paketbau verwendet

## 2.2 Projektspezifische Build-Abhaengigkeiten

Zusätzlich muessen die in `debian/control` definierten Build-Depends installierbar sein. Fuer dieses Projekt sind das:

```bash
sudo apt install cmake pkg-config qt6-base-dev qt6-declarative-dev qt6-tools-dev libmpv-dev
```

Diese Pakete sind fuer CoverFlowMP konkret noetig:

- `cmake`: Buildsystem
- `pkg-config`: findet `mpv` und weitere Build-Metadaten
- `qt6-base-dev`: Qt-Core/Gui-Basis
- `qt6-declarative-dev`: Qt Quick / QML Entwicklung
- `qt6-tools-dev`: Qt-Werkzeuge fuer den Build
- `libmpv-dev`: Entwicklungsdateien fuer libmpv

## 2.3 Sinnvolle Runtime-Pakete zum lokalen Testen

Fuer den eigentlichen Paketbau sind sie nicht zwingend erforderlich, fuer einen lokalen Starttest des gebauten Programms aber sehr sinnvoll:

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

Diese Pakete tauchen bereits als Laufzeitabhaengigkeiten fuer das Binärpaket auf.

## 2.4 Alles in einem Installationsschritt

Wenn ein neues Debian/Ubuntu-System fuer dieses Projekt vorbereitet werden soll, ist diese Kombination ein guter Start:

```bash
sudo apt update
sudo apt install \
  build-essential \
  devscripts \
  dpkg-dev \
  debhelper \
  fakeroot \
  cmake \
  pkg-config \
  qt6-base-dev \
  qt6-declarative-dev \
  qt6-tools-dev \
  libmpv-dev
```

Optional fuer lokale Laufzeittests zusaetzlich:

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

## 3. Bedeutung der Build-Schrittfolge

Hier ist die von dir genannte Folge noch einmal, jetzt mit Erklaerung:

```bash
debuild -- clean
rm -rf build obj-* debian/coverflowmp
dch -v x.y.z "Version info"
dpkg-buildpackage -us -uc -b
```

## 3.1 `debuild -- clean`

Dieser Befehl fuehrt den Debian-Clean-Schritt aus.

Praktisch bedeutet das in diesem Projekt:

- Debhelper startet den Clean-Ablauf ueber `debian/rules`
- CMake-Artefakte, die ueber den Debian-Build entstanden sind, werden entfernt
- der Baum wird moeglichst auf einen sauberen Paketbau-Zustand zurueckgesetzt

Warum das sinnvoll ist:

- alte Paketierungsreste stoeren den naechsten Build nicht
- veraltete Debhelper-Artefakte werden entfernt
- CMake-Konfigurationen aus aelteren Buildlaeufen bleiben nicht versehentlich erhalten

## 3.2 `rm -rf build obj-* debian/coverflowmp`

Dieser Schritt entfernt zusaetzliche Build-Reste, die ausserhalb des normalen Debhelper-Cleans liegen koennen.

In diesem Projekt ist das besonders relevant fuer:

- `build`
- `obj-*`

Wichtig fuer dieses Repo:

- Das Projekt verwendet aktuell tatsaechlich ein Verzeichnis wie `obj-x86_64-linux-gnu`.
- Deshalb ist `obj-*` hier ein sinnvoller Teil der Aufraeumlogik.
- Das Paket-Staging-Verzeichnis waere fuer dieses Projekt normalerweise `debian/coverflowmp`.

## 3.3 `dch -v x.y.z "Version info"`

Mit diesem Schritt erstellst du einen neuen Changelog-Eintrag und setzt die Debian-Version.

Beispiel:

```bash
dch -v 1.2.5 "Fix skip import behavior and packaging updates"
```

Wichtig:

- Der Paketname im Kopf von `debian/changelog` muss `coverflowmp` bleiben.
- Die Versionsnummer aus `debian/changelog` ist die fuer Debian massgebliche Paketversion.
- Bei einem nativen Paket gibt es keinen separaten Debian-Revisionsteil wie `-1`, ausser du entscheidest dich bewusst dafuer.

Empfehlung fuer dieses Projekt:

- Die Versionsnummer in `debian/changelog`
- die Projektversion in `CMakeLists.txt`
- und gegebenenfalls Release-Hinweise

sollten moeglichst zusammenpassen, damit keine Verwirrung zwischen Programmversion und Paketversion entsteht.

## 3.4 `dpkg-buildpackage -us -uc -b`

Das ist der eigentliche Paketbau.

Bedeutung der Optionen:

- `-us`: Source-Paket nicht signieren
- `-uc`: `.changes`-Datei nicht signieren
- `-b`: nur Binärpakete bauen

Was dabei passiert:

1. Debhelper liest `debian/control`, `debian/rules` und `debian/changelog`.
2. CMake konfiguriert das Projekt.
3. Das Binary wird gebaut.
4. `cmake --install` installiert in das Debian-Staging.
5. Debhelper erzeugt daraus das `.deb`.

Die resultierenden Dateien liegen ueblicherweise im Elternverzeichnis des Projekts, also eine Ebene oberhalb der Projektwurzel.

Typische Ergebnisse sind:

- `../coverflowmp_<version>_<arch>.deb`
- `../coverflowmp-dbgsym_<version>_<arch>.deb`
- `../coverflowmp_<version>_<arch>.buildinfo`
- `../coverflowmp_<version>_<arch>.changes`

## 4. Was im Projekt gepflegt werden muss, damit der Ablauf dauerhaft stabil bleibt

Damit diese Build-Sequenz auch spaeter noch funktioniert, sollten bei Aenderungen am Projekt immer diese Punkte mitgepflegt werden.

### 4.1 Bei neuen C++- oder Qt-Abhaengigkeiten

Wenn neue Bibliotheken ins Projekt kommen:

- `find_package(...)` in `CMakeLists.txt` anpassen
- `target_link_libraries(...)` anpassen
- `debian/control` unter `Build-Depends` ergaenzen

### 4.2 Bei neuen QML-Modulen

Wenn neue QML-Imports dazukommen:

- pruefen, welche Debian-Runtime-Pakete dafuer noetig sind
- `debian/control` bei `Depends` ergaenzen
- falls noetig auch die CPack-Runtime-Abhaengigkeiten in `CMakeLists.txt` aktualisieren

Das ist fuer dieses Projekt wichtig, weil QML-Abhaengigkeiten nicht immer automatisch aus ELF-Links erkannt werden.

### 4.3 Bei neuen installierten Dateien

Wenn neue systemweite Dateien hinzukommen, zum Beispiel:

- weitere Icons
- AppStream-Metadaten
- Udev-Regeln
- Hilfsdateien

dann muessen diese entweder:

- per `install(...)` in `CMakeLists.txt` aufgenommen werden

oder

- explizit ueber Debian-Dateien wie `debian/install` paketiert werden.

### 4.4 Bei Paketnamen-Aenderungen

Wenn `coverflowmp` umbenannt wird, muessen mindestens diese Stellen geprueft werden:

- `debian/control`
- `debian/changelog`
- Desktop-Dateiname und Icon-Zielname
- eventuelle Cleanup-Pfade unter `debian/`
- Dokumentation wie diese Datei

## 5. Empfohlener Praxisablauf fuer dieses Projekt

Ein praxistauglicher Ablauf fuer CoverFlowMP sieht so aus:

1. Im Projektwurzelverzeichnis arbeiten.
2. Sicherstellen, dass alle benoetigten Debian- und Qt/mpv-Buildpakete installiert sind.
3. Vorhandene Buildreste entfernen.
4. Changelog-Version erhoehen.
5. Paket bauen.
6. Das erzeugte `.deb` im Elternverzeichnis pruefen.
7. Optional das Paket lokal installieren und starten.

Beispiel:

```bash
debuild -- clean
rm -rf build obj-* debian/coverflowmp
dch -v 1.2.5 "Describe the changes here"
dpkg-buildpackage -us -uc -b
```

Optionaler Test:

```bash
sudo dpkg -i ../coverflowmp_1.2.5_amd64.deb
coverflowmp
```

## 6. Kurzfazit

Fuer dieses Projekt funktioniert die genannte Debian-Paketierung genau dann sauber, wenn:

- das Projekt aus der Wurzel heraus gebaut wird
- `debian/` konsistent zu `coverflowmp` gepflegt ist
- `CMakeLists.txt` alle zu paketierenden Dateien per `install(...)` erfasst
- die Debian-Toolchain installiert ist
- die Build-Depends aus `debian/control` auf dem System vorhanden sind

Der aktuelle Projektstand erfuellt die wesentlichen technischen Voraussetzungen bereits.
