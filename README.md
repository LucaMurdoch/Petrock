# PetRock

PetRock is a tiny GNOME app about looking after a pet that does absolutely nothing. Compliment Pebble, enjoy rotating rock facts, and admire a custom GDK paintable that glows brighter the kinder you are.

## Local development

```bash
meson setup build
meson compile -C build
GSETTINGS_SCHEMA_DIR=$PWD/build/data build/src/petrock
```

Translations can be updated with:

```bash
meson compile -C build petrock-pot
```

## Flatpak/Flathub workflow

1. Make sure `org.lucamurdoch.PetRock.yml` describes the latest release (runtime version, finish args, sources, etc.).
2. Build and run the sandboxed app:
   ```bash
   flatpak-builder --user --install --force-clean flatpak-build org.lucamurdoch.PetRock.yml
   flatpak run org.lucamurdoch.PetRock
   ```
3. Validate the metadata before submitting to Flathub:
   ```bash
   appstreamcli validate data/org.lucamurdoch.PetRock.metainfo.xml
   desktop-file-validate data/org.lucamurdoch.PetRock.desktop
   ```
4. Capture and update at least one 1280×720 screenshot in `data/screenshots/` and update the public URL inside the metainfo file. (Flathub requires the image to be published over HTTPS.)
5. Tag a release (for example `git tag -s 0.1.0 && git push origin 0.1.0`) and reference that tag in the Flathub manifest PR.

See <https://github.com/flathub/flathub/wiki/App-Submission> for the full checklist.
