# OpenLTC

**Prototype de générateur de timecode LTC à 25 images par seconde.** Le firmware tourne sur des cartes LOLIN S2 Mini (ESP32-S2). Une carte émet une référence de temps et les récepteurs recalibrent leur compteur via ESP-NOW. Chaque carte génère sa propre sortie LTC.

## Fonctionnement

- Le **maître** génère le timecode et diffuse sa position environ une fois par seconde.
- Un **récepteur** maintient son compteur local et applique les positions reçues à une frontière de trame.
- La génération utilise le **DAC de l’ESP32-S2** sur GPIO17 et un échantillonnage nominal de **48 kHz**. Le moniteur série affiche l’adresse MAC et le timecode au format `HH:MM:SS:FF`.

**État du projet** · Prototype de firmware. La dérive entre cartes, la gigue et la compatibilité avec des entrées LTC de tournage restent à mesurer. Le niveau électrique et le couplage de la sortie doivent être vérifiés avant de raccorder une caméra ou un enregistreur.

## Compiler le firmware

1. Installer PlatformIO et utiliser une carte **LOLIN S2 Mini**. L’environnement et la version de la plateforme Arduino ESP32 sont déclarés dans [`platformio.ini`](fw/LTC_sync/platformio.ini).
2. Dans [`include/config.h`](fw/LTC_sync/include/config.h), activer un seul rôle, `ROLE_MASTER` ou `ROLE_SLAVE`. Le dépôt sélectionne le rôle récepteur par défaut.
3. Depuis la racine du dépôt, compiler puis, pour flasher la carte connectée, lancer la deuxième commande.

```sh
pio run -d fw/LTC_sync -e lolin_s2_mini
pio run -d fw/LTC_sync -e lolin_s2_mini -t upload
```

Le débit du moniteur série est de **115200 bauds**.

## Sources

| Fichier | Rôle |
| --- | --- |
| [`main.cpp`](fw/LTC_sync/src/main.cpp) | Initialisation et diagnostic série |
| [`ltc.cpp`](fw/LTC_sync/src/ltc.cpp) | Trames LTC, horloge et sortie DAC |
| [`sync.cpp`](fw/LTC_sync/src/sync.cpp) | Liaison ESP-NOW entre maître et récepteurs |
