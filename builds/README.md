# Pre-built Spotify Connect ACAP Packages

Kant-en-klare ACAP-pakketten (`.eap`) voor alle Axis netwerkspeakers, één map per
chipset. Elke map heeft een `MODELS.md` met de modellen die erbij horen en de
AXIS OS-versie die nodig is.

## Welk pakket heb ik nodig?

| Model | Chipset | Map | AXIS OS |
|---|---|---|---|
| AXIS C1110-E | i.MX 8M Nano | [`imx8m-nano/`](imx8m-nano/MODELS.md) | 12.1+ |
| AXIS C1111-E | i.MX 8M Nano | [`imx8m-nano/`](imx8m-nano/MODELS.md) | 12.1+ |
| AXIS C1210-E | i.MX 6SoloX | [`imx6sx/`](imx6sx/MODELS.md) | 12.1+ |
| AXIS C1211-E | i.MX 6SoloX | [`imx6sx/`](imx6sx/MODELS.md) | 12.1+ |
| AXIS C1310-E | i.MX 6ULL | [`imx6ull/`](imx6ull/MODELS.md) | 11.11 LTS |
| AXIS C1310-E Mk II | i.MX 8M Nano | [`imx8m-nano/`](imx8m-nano/MODELS.md) | 12.1+ |
| AXIS C1410 | i.MX 6ULL | [`imx6ull/`](imx6ull/MODELS.md) | 11.11 LTS |
| AXIS C1410 Mk II | i.MX 8M Nano | [`imx8m-nano/`](imx8m-nano/MODELS.md) | 12.1+ |
| AXIS C1510 | i.MX 6SoloX | [`imx6sx/`](imx6sx/MODELS.md) | 12.1+ |
| AXIS C1511 | i.MX 6SoloX | [`imx6sx/`](imx6sx/MODELS.md) | 12.1+ |
| AXIS C1610-VE | i.MX 6SoloX | [`imx6sx/`](imx6sx/MODELS.md) | 12.1+ |
| AXIS C1710 | i.MX 8M Mini | [`imx8m-mini/`](imx8m-mini/MODELS.md) | 12.1+ |
| AXIS C1720 | i.MX 8M Mini | [`imx8m-mini/`](imx8m-mini/MODELS.md) | 12.1+ |
| AXIS C1004-E | ARTPEC-5 | [`artpec-5/`](artpec-5/MODELS.md) | geen build |
| AXIS C2005 | ARTPEC-5 | [`artpec-5/`](artpec-5/MODELS.md) | geen build |
| AXIS C3003-E | ARTPEC-5 | [`artpec-5/`](artpec-5/MODELS.md) | geen build |

Bron voor de chipset per model: de productdatabase van AXIS Site Designer
(veld `platform`), opgehaald op 2026-11-13.

**Andere Axis-apparaten** (geen speaker), zoals de AXIS I8116-E intercom
(ARTPEC-8): gebruik het `aarch64`-pakket uit `imx8m-nano/`. ARTPEC-6/-7-apparaten
gebruiken het `armv7hf`-pakket uit `imx6sx/`.

## AXIS OS 13

Axis meldt dat ACAP-apps voor AXIS OS 13 opnieuw gecompileerd moeten worden. De
SDK 13 is nog alpha; deze pakketten zijn met SDK 12.1 gebouwd en **niet getest op
AXIS OS 13**. Blijf op AXIS OS 12.11 LTS tot er een SDK 13-build is.

## Installatie

1. Log in op de webinterface van de speaker.
2. Ga naar **Apps** en klik op **Add app**.
3. Upload het `.eap`-bestand uit de map van jouw chipset.
4. Start de app en stel de Spotify-naam in. Zie de hoofd-README voor de stappen in
   Audio Manager Edge.

## Opnieuw bouwen

`./build_acap.ps1` in de hoofdmap bouwt alle pakketten (Docker vereist) en zet ze
in de juiste mappen.
