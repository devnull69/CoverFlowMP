# CoverFlowMP

CoverFlowMP ist ein videobasiertes Medien-Center für Linux-Systeme mit einfacher Tastatursteuerung. Die Bedienung ist in wenige klare Modi aufgeteilt:

- `Browser-Modus`: Ordner und Videos durchblättern
- `Play-Modus`: Video läuft
- `Pause-Modus`: Video steht, zusätzliche Feinnavigation und Bearbeitung sind möglich

## Starten

Das Programm kann optional mit einem Root-Video-Ordner gestartet werden:

```bash
CoverFlowMPApp /pfad/zum/video-root
```

Wenn kein Parameter angegeben wird, verwendet CoverFlowMP immer `~/Videos`.

## Dateien finden

Für ein Video `MeinFilm.mp4` werden zusätzliche Dateien im gleichen Ordner am schnellsten gefunden:

- Cover-Datei: `MeinFilm.jpg`
- Skip-Datei: `MeinFilm_skip.json`

Für Ordner kann ein eigenes Ordner-Cover hinterlegt werden:

- Ordner-Cover: `folder.jpg` innerhalb des jeweiligen Ordners

Skip-Dateien werden bei Bedarf zusätzlich auch in `~/Downloads` gesucht.

Beispiel für eine `*_skip.json` Datei:

```json
{
  "video_name": "MeinFilm",
  "source_file": "MeinFilm.mp4",
  "skip_ranges": [
    {
      "start": 12.5,
      "end": 48.0
    },
    {
      "start": 305.2,
      "end": 331.7
    }
  ]
}
```

## Browser-Modus

Im Browser-Modus bewegst du dich durch Ordner und Videos im CoverFlow.

### Navigation

- `Links` / `Rechts`: Auswahl im CoverFlow bewegen
- `Enter`: Ausgewähltes Element öffnen
- `Esc`: Eine Ebene nach oben gehen, falls `..` sichtbar ist; im Root-Ordner die App beenden

### Öffnen

- `Enter` auf einem Ordner: Ordner öffnen
- `Enter` auf `..`: Eine Ebene höher gehen
- `Enter` auf einem Video: Wiedergabe starten

### Zusätzliche Funktionen

- `0`: Ausgewähltes Video löschen oder einen leeren Ordner beziehungsweise einen Ordner nur mit `folder.jpg` löschen, nach Sicherheitsabfrage
- `F`: Fast-Modus umschalten
- `R`: Resume- und Skip-Datenbank komplett zurücksetzen, nach Sicherheitsabfrage

Hinweis zur Löschfunktion:

- Beim Löschen eines Videos werden zusätzlich die lokale Cover-Datei `Dateiname.jpg` und die lokale Skip-Datei `Dateiname_skip.json` im selben Ordner mit entfernt, falls vorhanden.
- Ordner lassen sich nur löschen, wenn sie leer sind oder ausschließlich die Datei `folder.jpg` enthalten. Vor dem Löschen eines solchen Ordners wird `folder.jpg` mit entfernt. Für Ordner erscheint dann die Sicherheitsabfrage `Diesen Ordner wirklich loeschen?`.

## Play-Modus

Im Play-Modus läuft das Video normal.

### Wiedergabe

- `Leertaste`: Pause ein/aus
- `B`: Zurück zum Browser

### Springen

- `Links`: 10 Sekunden zurück
- `Rechts`: 10 Sekunden vor
- `Shift` + `Links`: 5 Sekunden zurück
- `Shift` + `Rechts`: 5 Sekunden vor
- `Hoch`: 1 Minute vor
- `Runter`: 1 Minute zurück

Die Sprünge berücksichtigen immer die Videogrenzen.
Im Normalmodus werden vorhandene Skip-Bereiche sowohl bei der normalen Wiedergabe als auch bei der Sprung-Navigation automatisch übersprungen.

## Pause-Modus

Im Pause-Modus ist das Video angehalten. Zusätzlich erscheint ein Overlay mit Fortschritt, Position und weiteren Informationen.

Solange ein Video im Player geöffnet ist, fordert CoverFlowMP das System an, keinen Bildschirmschoner, keine automatische Bildschirmabschaltung und nach Möglichkeit keine Idle-Sperre zu aktivieren. Das gilt sowohl im Play- als auch im Pause-Modus.

### Wiedergabe und Navigation

- `Leertaste`: Wiedergabe fortsetzen
- `B`: Zurück zum Browser
- `Links`: 10 Sekunden zurück
- `Rechts`: 10 Sekunden vor
- `Shift` + `Links`: 5 Sekunden zurück
- `Shift` + `Rechts`: 5 Sekunden vor
- `Hoch`: 1 Minute vor
- `Runter`: 1 Minute zurück
- `+`: Ein Frame vor
- `-`: Ein Frame zurück
- `E`: Zum letzten Frame springen, ohne das Video zu beenden

Wichtig:

- Im Pause-Modus beendet Cursor-Navigation das Video nie. Am Anfang oder Ende bleibt das Bild einfach auf dem ersten bzw. letzten Frame stehen.
- Im Normalmodus werden vorhandene Skip-Bereiche auch im Pause-Modus bei der Sprung-Navigation automatisch berücksichtigt.

### Skip-Bereiche bearbeiten

Skip-Bereiche stehen nur im Normalmodus zur Verfügung. Im Fast-Modus sind sie deaktiviert.

Wenn fuer ein Video noch keine Skip-Bereiche gespeichert sind, aber im gleichen Ordner bereits eine passende `*_skip.json` liegt, startet das Video im Normalmodus kurz in den Pause-Modus und fragt den Import an.

- `S`: Skip-Bereich setzen
  Beim ersten Druck wird der Startpunkt gesetzt, beim zweiten Druck der Endpunkt.
- `C`: Einen begonnenen, aber noch nicht abgeschlossenen Skip-Bereich verwerfen
- `Entf` / `Backspace`: Alle gespeicherten Skip-Bereiche dieses Videos löschen, nach Sicherheitsabfrage
- `X`: Skip-Bereiche dieses Videos als Datei exportieren
- `I`: Skip-Bereiche dieses Videos importieren

### Audio-Verzögerung anpassen

- `A`: Audio-Delay-Modus öffnen
- Im Audio-Delay-Modus bewirken `Links` und `Rechts` jeweils `50 ms` früher bzw. später
- `Esc`: Audio-Delay-Modus verlassen

## Resume-Dialog

Wenn es zu einem Video eine gespeicherte Wiedergabeposition gibt, erscheint beim Start ein Resume-Dialog.

- `Hoch` / `Runter`: Zwischen `Weiter` und `Von vorne` wählen
- `Enter`: Auswahl bestätigen

## Dialogbedienung

Die Dialogsteuerung ist im Browser und im Player grundsätzlich gleich:

- `Hoch` / `Runter`: Zwischen `JA` und `NEIN` wechseln, falls Auswahl vorhanden
- `Enter`: Auswahl bestätigen
- `Esc`: Dialog schließen, wenn erlaubt
- `B`: Dialog schließen, wenn erlaubt

Das gilt unter anderem für:

- Löschdialog im Browser
- Rücksetzdialog im Browser
- Dialog zum Löschen aller Skip-Bereiche eines Videos im Pause-Modus
- Meldungsdialoge im Player

Besonderheiten:

- Beim Dialog zum Löschen von Skip-Bereichen ist `NEIN` standardmäßig vorausgewählt.
- Der Resume-Dialog verwendet nur `Hoch`, `Runter` und `Enter`.

## Fast-Modus

Der Fast-Modus ist für schnelles Anschauen gedacht.

- Skip-Bereiche werden nicht automatisch angewendet
- Das Setzen, Loeschen, Importieren und Exportieren von Skip-Bereichen ist deaktiviert
- Bereits gespeicherte Skip-Bereiche bleiben erhalten und können später im Normalmodus wieder genutzt werden
