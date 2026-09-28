# Earth Chorus — by NACHE

Plugin de chorus (VST3 / AU / Standalone) inspiré du Chorus Dimension D d'Arturia et du chorus du Juno-106.
Construit avec [JUCE 8](https://juce.com) (téléchargé automatiquement par CMake).

## Contrôles
- **MODE** : 4 modes (I, II, I+II, DIM) inspirés du Juno-106 + un mode "Dimension" à 2 voies par canal
- **SPEED** : vitesse du LFO (0.05 – 10 Hz)
- **DEPTH** : profondeur de modulation
- **SUPER FX** : spatialisation profonde (diffusion allpass, premières réflexions croisées, élargissement Mid/Side, orbite stéréo)
- **Globe / anneau** : Dry/Wet (le jour/nuit du globe suit la valeur ; la rotation suit SPEED)
- **Interrupteur** en haut à droite, **VU-mètre** au-dessus du titre
- **Menu de presets** en bas : 25 presets d'usine, sauvegarde de presets utilisateur, flèches précédent/suivant
- **Double-clic sur le titre** : menu caché (version, date, lien)

## Compiler sur GitHub (sans rien installer)
1. Créez un dépôt GitHub (ex. `EarthChorus`), vide.
2. Décompressez l'archive et envoyez **tout le contenu** dans le dépôt (y compris le dossier caché `.github`).
   En ligne de commande :
   ```
   cd EarthChorus
   git init && git add . && git commit -m "Earth Chorus 1.0"
   git branch -M main
   git remote add origin https://github.com/VOTRE_NOM/EarthChorus.git
   git push -u origin main
   ```
3. Onglet **Actions** → workflow **Build Earth Chorus** (il démarre au push ; sinon *Run workflow*).
4. Une fois terminé (~10-15 min), ouvrez l'exécution et téléchargez les artefacts en bas de page :
   `EarthChorus-Windows`, `EarthChorus-macOS`, `EarthChorus-Linux`.

## Installation
- **Windows** : copier `Earth Chorus.vst3` dans `C:\Program Files\Common Files\VST3`
- **macOS** : `Earth Chorus.vst3` → `~/Library/Audio/Plug-Ins/VST3` ; `Earth Chorus.component` → `~/Library/Audio/Plug-Ins/Components`.
  Le plugin n'est pas signé : `xattr -cr "~/Library/Audio/Plug-Ins/VST3/Earth Chorus.vst3"`
- **Linux** : `~/.vst3/`
Puis relancez le scan de plugins de votre DAW.

## Compiler en local (optionnel)
```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
Résultat : `build/EarthChorus_artefacts/Release/`

## Personnalisation
- **Police "Thriller"** : placez un fichier `Thriller.ttf` (dont vous avez la licence) dans `Resources/`, elle sera intégrée automatiquement. Sans ce fichier, une police serif grasse est utilisée.
- **Titre / version / date / lien** : constantes en haut de `Source/PluginEditor.cpp` (`kTitle`, `kVersion`, `kDate`, `kLinkUrl`).
- **Presets d'usine** : `Source/Presets.h`.
- Presets utilisateur enregistrés dans `.../NACHE/Earth Chorus/Presets/` (dossier de données d'application).

## Licence
JUCE est distribué sous AGPLv3 ou licence commerciale/gratuite JUCE : vérifiez que votre usage (distribution, vente) est conforme.
