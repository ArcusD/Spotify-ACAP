# NXP i.MX 6ULL (armv7hf)

**Pakket:** `Spotify_Connect_1_0_0_armv7hf.eap`
**Gebouwd met:** ACAP Native SDK 1.15.4 (armv7hf)
**Vereist:** AXIS OS 11.11 LTS

## Ondersteunde modellen

| Model | Type | Artikelnummer |
|---|---|---|
| AXIS C1310-E | Network Horn Speaker (eerste generatie) | 01796-001, 01796-014 |
| AXIS C1410 | Network Mini Speaker (eerste generatie) | 01916-001 |

## Let op

- Deze modellen zijn uitgefaseerd en krijgen geen AXIS OS 12; de laatste versie is
  11.11 LTS. Daarom is dit pakket met de oudere SDK gebouwd. Gebruik **niet** het
  pakket uit `imx6sx/`: dat vereist AXIS OS 12.1 en installeert hier niet.
- De opvolgers **C1310-E Mk II** en **C1410 Mk II** hebben een andere chip; zie
  `imx8m-nano/`.
- Nog **niet getest** op een echt apparaat. Controleer of Audio Manager Edge op
  het apparaat beschikbaar is; de app meldt zich daar als audiobron aan.
