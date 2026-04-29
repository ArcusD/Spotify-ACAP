# Pre-built Spotify Connect ACAP Packages

Deze map bevat de kant-en-klare ACAP pakketten (`.eap` bestanden) voor Spotify Connect, gecompileerd voor Axis OS 12.

## Welke versie heb ik nodig?

Axis netwerkcamera's en intercoms gebruiken verschillende processoren. Kies het juiste bestand op basis van je hardware:

1. **`Spotify_Connect_1_0_0_aarch64.eap`**
   - **Voor:** Moderne Axis apparaten (met **ARTPEC-8** of **Ambarella CV25** chips).
   - **Voorbeeld:** Axis I8116 Intercom, Axis M32-serie.

2. **`Spotify_Connect_1_0_0_armv7hf.eap`**
   - **Voor:** Oudere generatie Axis apparaten (met **ARTPEC-6** of **ARTPEC-7** chips).
   - **Voorbeeld:** Axis Q16-serie, oudere audiospeakers.

## Installatie Instructies
1. Log in op de web-interface van je Axis camera.
2. Ga naar het **Apps** paneel.
3. Klik op **Add app** (of '+' icoontje) en upload het `.eap` bestand dat bij jouw processor past.
4. Klik op de applicatie in de lijst om hem te starten en configureer de gewenste Spotify-naam in de instellingen.

> **WIP (Work In Progress):** Mocht de applicatie niet werken op jouw specifieke apparaat, check dan de "View Debug Log" op de applicatie-pagina voor foutmeldingen, en controleer of je de juiste architectuur hebt geïnstalleerd.
