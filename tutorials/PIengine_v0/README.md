# PIengine_v0 — Cas moteur 4 soupapes (dérivé de tcc3)

> **Statut : préparation.** Ce cas est une copie de `tutorials/tcc3` (OpenFOAM-14 / AATE)
> en cours d'adaptation vers un moteur à **4 soupapes**. Ce document reprend l'ensemble
> des questions posées pendant l'étude du framework et leurs réponses, puis servira de
> **journal des modifications** au fur et à mesure de l'adaptation.

## Sommaire

1. [Objectif du cas](#1-objectif-du-cas)
2. [Comment la géométrie est construite à partir des STL](#2-comment-la-géométrie-est-construite-à-partir-des-stl)
3. [Pipeline de maillage (Allmesh)](#3-pipeline-de-maillage-allmesh)
4. [Inventaire du cas tcc3 et correspondance des STL](#4-inventaire-du-cas-tcc3-et-correspondance-des-stl)
5. [Les 4 conditions aux limites](#5-les-4-conditions-aux-limites)
6. [Paramètres cinématiques : où est quoi](#6-paramètres-cinématiques--où-est-quoi)
7. [insidePoints : à quoi ça sert](#7-insidepoints--à-quoi-ça-sert)
8. [Calage des soupapes : les CSV](#8-calage-des-soupapes--les-csv)
9. [Les dossiers polyMesh de constant/meshes](#9-les-dossiers-polymesh-de-constantmeshes)
10. [Paramètres à revoir lors d'un changement de géométrie](#10-paramètres-à-revoir-lors-dun-changement-de-géométrie)
11. [zonesGenerator en détail](#11-zonesgenerator-en-détail)
12. [Pourquoi 3 STL par siège (dans tcc3) et choix PIengine_v0](#12-pourquoi-3-stl-par-siège-dans-tcc3-et-choix-piengine_v0)
13. [Position CAO des soupapes et initialPosition](#13-position-cao-des-soupapes-et-initialposition)
14. [Les supportSurfaces (ValveCut)](#14-les-supportsurfaces-valvecut)
15. [Ajouter des pièces / CL non conventionnelles (entretoise)](#15-ajouter-des-pièces--cl-non-conventionnelles-entretoise)
16. [Le cas imperialcollege (modèle minimal)](#16-le-cas-imperialcollege-modèle-minimal)
17. [Bilan des fichiers STL pour PIengine_v0](#17-bilan-des-fichiers-stl-pour-piengine_v0)
18. [Modifications effectuées (journal)](#18-modifications-effectuées-journal)
19. [TODO avant premier maillage](#19-todo-avant-premier-maillage)

---

## 1. Objectif du cas

Dériver le cas `tcc3` (TCC-III, moteur 4-temps, 2 soupapes) vers un moteur à
**4 soupapes** (2 admission + 2 échappement), en conservant :

- le **piston** (`PN_Topland`, `PN_Crevice`) et la **chemise** (`LN_Wall`) de tcc3,
- la méthodologie de remaillage AATE (multi-maillages + interpolation `meshToMesh`).

Décisions de découpage CAO (v0) :

- soupapes : **5 fichiers par soupape** (`Stem`, `Angle`, `Seat`, `Bottom`,
  `Bottom_Side`), comme tcc3 — voir §17 ;
- sièges (dans la culasse, côté conduit) : **1 seule tranche** par soupape
  (tcc3 en utilise 3, voir §12) → `Seat_1`, `Seat_2` par conduit ;
- conduits admission/échappement : **4 fichiers par conduit** (`Inlet`/`Outlet`,
  `Wall` commun aux 2 soupapes du conduit, `Seat_1`, `Seat_2`) ;
- **bougie** : paroi plate fusionnée dans la culasse en v0 (pas de STL dédié) ;
- soupapes modélisées **fermées** dans la CAO (`initialPosition 0` partout, voir §13).

## 2. Comment la géométrie est construite à partir des STL

**Il n'y a pas de dictionnaire classique référençant les STL** (pas de bloc
`geometry` dans un `snappyHexMeshDict` écrit à la main). Tout est piloté par le
dictionnaire **`constant/engineDict`** :

- les STL sont **découverts automatiquement** dans `constant/geometry/` par
  balayage du répertoire (`findSurfaceFiles()` —
  `src/preProcessing/findSurfaceFiles/findSurfaceFiles.H` : tout fichier lisible
  par `meshedSurface::canRead()`, donc pas seulement des STL) ;
- chaque patch est défini dans `engineDict` par un bloc `patches { <nom> { ... } }`
  avec le mot-clé **`surfaces (nom_sans_extension ...)`** (regex autorisées) ;
- le nom du bloc = nom du patch OpenFOAM final ; les STL listés deviennent ses faces ;
- chaque bloc définit aussi `geometryMotion` (mouvement) et `meshControl`
  (raffinement, couches limites).

Le `snappyHexMeshDict` complet (avec le bloc `geometry` et les références STL)
est **généré automatiquement** par l'utilitaire `engineMeshConfig` pour chaque
instant de remaillage — c'est pour cela qu'on ne le trouve pas dans le cas.
Seul `system/snappyHexMeshDict.cfg` (contrôles de qualité/snap/layers) est écrit
à la main.

## 3. Pipeline de maillage (Allmesh)

```
tar -xzf constant/geometry.tar.gz -C constant     (extraction des STL)
predictRemeshInstants   → calcule les instants de remaillage (constant/meshTimes)
moveSurfaces            → duplique et transforme les STL par instant
                          vers constant/meshes/<t>/constant/geometry/
engineMeshConfig        → écrit blockMeshDict + snappyHexMeshDict_main + meshQualityDict
                          pour chaque instant dans constant/meshes/<t>/system/
generateMesh -mesh <t>  → blockMesh + snappyHexMesh pour chaque instant
                          → constant/meshes/<t>/polyMesh/
```

Pendant le calcul (`Allrun`) : le mover `multiValveEngine` déforme le maillage
entre remaillages ; le topoChanger `meshToMesh` bascule sur le polyMesh suivant
en interpolant tous les champs.

## 4. Inventaire du cas tcc3 et correspondance des STL

Préfixes : **PN** = piston, **LN** = liner, **HD** = culasse (head),
**SP** = entretoise/spark plug, **IV** = intake valve, **EV** = exhaust valve,
**IP** = intake port, **EP** = exhaust port.

| STL | Pièce |
|---|---|
| `PN_Topland.stl` | Piston — couronne (translation vilebrequin-bielle) |
| `PN_Crevice.stl` | Piston — creux périphérique (raffiné niv. 7) |
| `LN_Wall.stl` | Chemise — scalingMotion (étirée avec le piston) |
| `HD_Wall.stl` | Culasse — face chambre |
| `SP_Wall.stl` | Bougie (paroi, raffinée niv. 4) |
| `IV_Stem.stl` | Soupape admission — tige (valveMotion) |
| `IV_Angle.stl` | Soupape admission — partie conique |
| `IV_Bottom.stl`, `IV_Bottom_Side.stl` | Soupape admission — face inf. + flanc |
| `IV_Seat.stl` | Soupape admission — portée sur la soupape |
| `EV_*` (idem IV) | Soupape échappement |
| `IP_Wall_Pipe.stl`, `IP_Plenum_*`, `IP_Pipe_Inlet.stl` | Conduit admission (parois) |
| `IP_Inlet.stl` | Bride d'entrée admission (patch inlet) |
| `IP_Seat_Top/Middle/Bottom.stl` | Siège admission dans la culasse — 3 bandes |
| `EP_*` (idem IP) | Conduit échappement + siège (3 bandes) + bride outlet |
| `supportSurfaces/*_ValveCut.stl` | Bouchons des volumes morts (voir §14) |

Autres fichiers du cas :

| Fichier | Rôle |
|---|---|
| `constant/engineDict` (`.coarse`/`.fine`) | Configuration centrale : patches, mouvements, raffinement |
| `constant/engineData/*Lift.txt` | Lois de levée soupapes (CAD → lift) |
| `constant/engineData/*Pressure.txt` | Pressions aux bornes (inlet/outlet) |
| `constant/engineData/chamberPressure.txt` | Pression chambre **expérimentale** (validation) |
| `constant/dynamicMeshDict` | Mover `multiValveEngine` + topoChanger `meshToMesh` |
| `constant/zonesGenerator` | Zones pour initialisation zonale + mover (§11) |
| `constant/momentumTransport` | Turbulence (k-ω SST) |
| `0.orig/{p,T,U,k,omega,nut,alphat,tracer}` | Champs initiaux (groupes de CL, §5) |
| `system/*` | Contrôles solveur + gabarits maillage |
| `remeshInstants.py`, `plot_scripts/`, `plotData.gnu` | Post-traitement |

## 5. Les 4 conditions aux limites

Les CL ne sont pas des patches individuels mais des **groupes** (`inGroups`)
déclarés dans `engineDict`, utilisés directement dans les champs `0.orig/*` :

| Groupe | Patches (tcc3) | U | p | T |
|---|---|---|---|---|
| `staticWall` | défaut global : ports, liner, seats, culasse, bougie | `noSlip` | `zeroGradient` | `fixedValue` 310,66 K |
| `movingWall` | `piston`, `iv_head`, `ev_head` | `movingWallVelocity` (vitesse locale auto) | `fixedFluxPressure` | = staticWall |
| `inlet` | `inlet` (IP_Inlet) | `pressureInletOutletVelocity` | `uniformTotalPressure` (CSV `intakePressure.txt`, CAD) | `uniformInletOutlet` |
| `outlet` | `outlet` (EP_Outlet) | = inlet | `uniformTotalPressure` (CSV `exhaustPressure.txt`) | `zeroGradient` |

Construction en 3 étapes : STL → `inGroups` dans `engineDict` → `boundaryField`
des champs. **Une seule CL déclarée pour N patches** : ajouter une paroi fixe
n'exige aucun changement dans `0.orig` (héritage du groupe `staticWall`).

Les `internalField` de `p`, `T`, `tracer` sont **zonaux** (chamber 18 bar/800 K,
intakePort 0,945 bar/317 K, exhaustPort 1 bar/520 K) via les zones de §11.

## 6. Paramètres cinématiques : où est quoi

| Paramètre | Fichier | Entrée |
|---|---|---|
| Course, bielle | `constant/engineDict` | `patches/piston/geometryMotion/motion` (`crankConnectingRodMotion`, `stroke 0.086`, `conRodLength 0.231` — tcc3) |
| Alésage | implicite | géométrie STL du liner |
| Levées soupapes | CSV `engineData/*Lift.txt` | référencés dans `patches/i?_*/geometryMotion/motion` (admission en m, échappement en mm avec `scale 0.001`) |
| Calage ouverture/fermeture | CSV de levée eux-mêmes | colonne d'angles (pas de mot-clé de phase, voir §8) |
| Régime | `system/controlDict` | `userTime { type engine; rpm 800; }` — tout le cas est en CAD, le rpm ne fait que convertir CAD ↔ secondes |
| Clarance morte PMH | `engineDict` | `remeshControl/clearance` (0,0095 m tcc3) |
| Pressions aux bornes | CSV `engineData/*Pressure.txt` | `0.orig/p` |
| Init par zone | `0.orig/{p,T}` | `internalField` zonal |

`dynamicMeshDict` réutilise les valeurs d'`engineDict` par références croisées
(`${engineDict!patches/piston/...}`) : **une seule source de vérité**.

## 7. insidePoints : à quoi ça sert

`engineDict/meshControl/insidePoints` = liste de points qui disent à snappyHexMesh
**quel(s) volume(s) fluide mailler** (équivalent multi-volumes du `locationInMesh`).

- tcc3 en a **3** : chambre + conduit admission + conduit échappement (les volumes
  sont déconnectés quand les soupapes sont fermées ; un seul point ne suffirait pas).
- Les points sont recopiés par `engineMeshConfig` dans le `snappyHexMeshDict_main`
  **de chaque instant** → ils doivent rester dans le fluide pour toutes les positions
  du piston (choisir des positions « sûres » : centre du cylindre à mi-course,
  centre de chaque conduit).
- Règle pour PIengine_v0 : un point par volume fluide déconnectable.

## 8. Calage des soupapes : les CSV

`valveMotion` n'accepte que `minLift`, `closureTolerance`, `dt` — **pas de
mot-clé de phase/offset**. La variable d'entrée étant le CAD :

> **Décaler le calage = décaler la colonne d'angles du CSV.**
> Avancer l'ouverture de 10° → soustraire 10 à tous les angles.

Idem pour les CSV de pression. Bonne pratique : générer les CSV par script
paramétrique (IVO, IVC, EVO, EVC, lifts max) plutôt que les éditer à la main.

Ce qui se recalcule automatiquement quand on change un calage :
`constant/meshTimes` (par `predictRemeshInstants`, détection des ouvertures/
fermetures), les instants de remaillage aux `forcedRemeshPositions` = `minLift`.

## 9. Les dossiers polyMesh de constant/meshes

Un `polyMesh` par instant de remaillage = un maillage complet pour la géométrie
déformée à cet instant (piston/soupapes/liner aux positions exactes). Rôle :

- entre deux remaillages : le maillage est **déformé** par le mover ;
- au remaillage : le topoChanger `meshToMesh` **bascule** sur le polyMesh
  pré-calculé suivant en **interpolant tous les champs** ;
- `Allrun` décompose chaque polyMesh à l'avance (`decomposePar -mesh {}`) pour
  un basculement parallèle instantané.

Les instants sont calculés par `predictRemeshInstants` à partir des
`remeshControl` de chaque pièce (tolérances piston, fermetures de soupapes aux
`forcedRemeshPositions`, points morts `forcedRemeshInstants`).

Conséquences : coût disque élevé (maillage × N domaines × N instants) ;
après tout changement de géométrie/raffinement, **purger `constant/meshes/`**
et relancer `Allmesh` (les anciens polyMesh deviennent incohérents).

## 10. Paramètres à revoir lors d'un changement de géométrie

Aucune recompilation nécessaire — tout est dictionnaires. Checklist :

**Obligatoires (échec sinon)** :
- `insidePoints` (positions des volumes fluides) ;
- blocs `patches/surfaces (...)` (noms réels des STL) ;
- `geometryMotion` par pièce mobile (axes, `initialPosition`, `minLift`) ;
- `clearance` (clarance morte au PMH réelle) ;
- `dynamicMeshDict` : `slidingPatches`, `linerPatches`, `frozenZones`/`movingZones` ;
- `zonesGenerator` (zones cohérentes avec la nouvelle géométrie).

**Recommandés** :
- `cellSize` (échelle globale), `regionRefinement` (coordonnées codées pour tcc3) ;
- `supportSurfaces` (ValveCut, voir §14) ;
- `layerControl` ; tolérances `remeshControl` (fréquence des remaillages).

**Procédure** : `surfaceCheck` sur chaque STL → adapter dicts → `Allclean` →
`Allmesh` → **`checkMesh` sur `constant/meshes/0/polyMesh` avant la boucle
complète** → ajuster → `Allrun`.

## 11. zonesGenerator en détail

`createZones` (OpenFOAM-14) lit `constant/zonesGenerator` et crée des zones sur
le maillage initial. Trois consommateurs :

1. **Initialisation zonale** des champs (`internalField zonal`) ;
2. **Mover** `multiValveEngine` (`frozenZones`/`movingZones` de `dynamicMeshDict`) ;
3. **Post-traitement** (moyennes par zone).

Blocs du zonesGenerator tcc3 :

| Zone | Type | Construction |
|---|---|---|
| `sparkPlug` | cellZone | cylindre analytique (axe bougie) |
| `chamber` | cellZone | `union` (cylindre hérité d'`engineDict` via `$engineDict!...`, bougie) |
| `exhaustPort` | cellZone | boîte |
| `intakePort` | cellZone | **`difference`** : tout − (chambre + échappement + bougie) |
| `cylinderHead` | pointZone | `cylinderHeadPoints`, `moveUpdate true` → `frozenZones` du mover (points ancrés) |
| `pistonBowl` | pointZone | `pistonBowlPoints` enveloppe `periodic` ±40° PMH avec `deactivate` → `movingZones`, pour contrôler la déformation près du PMH |

Techniques utiles à retenir : héritage `$engineDict!...` (une seule définition
géométrique pour raffinement + zone), union/différence (pas besoin d'épouser les
formes), périodicité temporelle (activation/désactivation par fenêtre CAD).

## 12. Pourquoi 3 STL par siège (dans tcc3) et choix PIengine_v0

Les 3 fichiers (`IP_Seat_Top/Middle/Bottom`) sont **3 tranches empilées d'un même
cône de siège** (mesuré : tranches contiguës z = 0,0006→0,0034 m, rayon croissant
0,014→0,0157, même axe) — pas des copies décalées. Objectif :

1. **Graduer le raffinement** autour de la fente de levée : la fente annulaire
   soupape/siège (quelques dixièmes de mm à l'ouverture) exige des cellules très
   fines, mais seulement près d'elle ; 3 bandes de niveaux croissants font la
   transition propre entre conduit grossier et orifice fin (snappy exige des
   transitions progressives).
2. Fournir une **cible propre** au `dynamicRefinement` (`numberOfCells 3`
   à travers la fente, recalculé à chaque remaillage).

**Choix PIengine_v0 : 1 seule tranche par siège** (simplicité CAO), avec un
`surfaceRefinement` uniforme élevé. Le raffinement dynamique côté soupape
(`IV*_Seat`, `dynamicRefinement`) continue de résoudre la fente. Revenir au
découpage en 3 bandes en v1 si le maillage aux ouvertures s'avère trop coûteux.

## 13. Position CAO des soupapes et initialPosition

Mécanisme (`valveMotion.C`, hérité de `translationMotion.C:46`) :

```
déplacement(t) = lift(t) − initialPosition       // initialPosition défaut = 0
```

Le STL doit être dessiné **à la levée déclarée par `initialPosition`** (la
valeur par défaut 0 s'applique si l'entrée est absente). Vérifié sur tcc3 :

- **Admission** : CAO **fermée** (face plane affrontée au siège), pas
  d'`initialPosition` → 0 par défaut ;
- **Échappement** : CAO ouverte de **0,95 mm** (mesuré sur STL), déclarée
  `initialPosition 9.5e-4`.

Les CSV donnent `lift(0°) = 0` pour les deux soupapes : à l'instant 0,
l'admission ne bouge pas (fermée), l'échappement est abaissée de 0,95 mm
(déplacement = 0 − 0,00095) → **les deux sont fermées au maillage de t=0**,
malgré des CAO de référence différentes. `initialPosition` n'est pas un réglage
physique : c'est la **déclaration de ce que représente la CAO**.

**Choix PIengine_v0 : 4 soupapes CAO en position fermée + `initialPosition 0`
déclaré explicitement partout** (auto-documentation), piston au PMH.

Pièges : CAO mi-levée inutile ; incohérence CAO/`initialPosition` = tout le
cycle décalé d'autant.

## 14. Les supportSurfaces (ValveCut)

Rôle : quand une soupape est **fermée**, il existe un **volume mort** entre la
tige et le guide de culasse (les guides ne sont pas modélisés en CAO). Le
`ValveCut.stl` est une **surface-bouchon artificielle** (disque/tronc de cône
posé juste au-dessus de la face de culasse, autour de la tige) sur laquelle le
maillage s'arrête. Elle :

- empêche snappy de mailler le volume mort inutile ;
- garantit l'**étanchéité** chambre/conduit aux fermetures (sinon risque de
  « fuite » topologique à travers la soupape fermée) ;
- est déclarée dans `engineDict` (`supportSurfaces { ... type triSurface; file
  "..."; }`), écrite dans le `snappyHexMeshDict_main` généré avec le `patchInfo`
  du patch soupape, et **suit la soupape** (translatée avec elle par `moveSurfaces`).

**PIengine_v0 : 4 ValveCut obligatoires** (un par soupape), fabriqués CAO
soupape fermée : disque/anneau perpendiculaire à l'axe, rayon ≥ guide de soupape,
juste au-dessus de la face de culasse (référence tcc3 : ils s'étendent ~0,5 à
~4 mm au-dessus de la face).

## 15. Ajouter des pièces / CL non conventionnelles (entretoise)

Ajouter une **pièce fixe** (ex. entretoise bloc/culasse) est trivial : 1 STL +
1 bloc dans `engineDict` (`surfaces`, `meshControl`, `inGroups` éventuel) —
aucune recompilation, pas de changement dans `0.orig` si elle reste dans
`staticWall`. Un `inGroups` dédié ne sert que pour une CL différente.

Pour une CL « spécifique », du plus simple au plus fort : BC standard
(`fixedValue`, `externalWallHeatFluxTemperature`...), moyennes par groupe,
`fvConstraints`/`fvModels`, **`codedFixedValue`/`codedMixed`** (C++ dans le
dictionnaire, sans recompiler AATE), nouvelle classe BC perso.

Limites : nouvelles **cinématiques** = nouvelle classe `geometryMotion` en C++
(3 types fournis : `translationMotion`, `scalingMotion`, `valveMotion`) ;
pas de couplage fluide-structure natif. tcc3 contient d'ailleurs déjà une
entretoise : `SP_Wall.stl` (patch `sparkplug`).

## 16. Le cas imperialcollege (modèle minimal)

Gabarit minimal AATE : **8 STL** (piston, liner, culasse, port, 3× soupape
**statique**, outlet), 4 blocs de patches. Soupapes sans `geometryMotion`
(banc d'essai), pas de supportSurfaces, pas de multi-instants de soupape.
Utile comme point de départ pour une géométrie simple ou des tests rapides.

## 17. Bilan des fichiers STL pour PIengine_v0

**Conservés de tcc3 (3)** :

| Fichier | Rôle |
|---|---|
| `PN_Topland.stl` | Piston — couronne (mobile) |
| `PN_Crevice.stl` | Piston — crevice (mobile) |
| `LN_Wall.stl` | Chemise (scaling) |

**À fournir (33)** :

| Catégorie | Fichiers | Détail |
|---|---|---|
| Culasse | `HD_Wall.stl` (1) | 4 puits de soupape, bougie en paroi plate intégrée (v0) |
| Soupapes | 4 soupapes × 5 fichiers = **20** : `IV1/IV2/EV1/EV2` `_Stem`, `_Angle`, `_Seat`, `_Bottom`, `_Bottom_Side` | **CAO fermées**, `initialPosition 0` ; 5 fichiers comme tcc3 (raffinements distincts tige/face) |
| Conduits admission | `IP_Inlet.stl`, `IP_Wall.stl`, `IP_Seat_1.stl`, `IP_Seat_2.stl` (**4**) | bride (patch inlet), mur **commun aux 2 soupapes**, 2 sièges à 1 tranche (paroi fixe) |
| Conduits échappement | `EP_Outlet.stl`, `EP_Wall.stl`, `EP_Seat_1.stl`, `EP_Seat_2.stl` (**4**) | bride (patch outlet), mur commun, 2 sièges à 1 tranche |
| ValveCut | `supportSurfaces/IV1/IV2/EV1/EV2_ValveCut.stl` (**4**) | bouchons des puits de tiges, CAO soupape fermée |

**Total : 36 STL** (3 conservés + 33 à fournir).

**Abandonnés/modifiés par rapport à tcc3** :

- `IP_Seat_Top/Middle/Bottom` → `IP_Seat_1/2` (3 tranches → 1 tranche
  par soupape, §12) ; idem `EP_Seat_*` ;
- `IP_Wall_Pipe`, `IP_Plenum_*`, `IP_Pipe_Inlet` → un seul `IP_Wall.stl`
  commun aux 2 soupapes d'admission ; idem côté échappement ;
- `SP_Wall.stl` : paroi plate intégrée à `HD_Wall.stl` (bougie en v0) ;
- renommage 2 → 4 soupapes : `IV_*` → `IV1_*`/`IV2_*`, `EV_*` → `EV1_*`/`EV2_*`.

**Nomenclature** : préfixes soupape `IV1/IV2/EV1/EV2` ; les regex
`"IV1_Seat.*"`, `"IP_Seat.*"` etc. dans `engineDict` matchent les fichiers.

**Dictionnaires à adapter** (détaillé §10) : `engineDict` (+`.coarse`/`.fine`),
`dynamicMeshDict` (bloc `valves` : 4 entrées), `engineData` (CSV de levée ×4,
pressions), `zonesGenerator` (sans bougie, boîtes ports élargies), `Allmesh`
(si géométrie livrée en clair au lieu du tar.gz).

## 18. Modifications effectuées (journal)

### 18.1 Correspondance fichiers réels ↔ rôles (noms déposés par l'utilisateur)

Les 36 STL déposés utilisent la nomenclature `IV_1_*` (avec underscore) et
`Steam` (au lieu de `Stem` — coquille conservée telle quelle dans les
fichiers, les dictionnaires référencent donc `IV_1_Steam`) :

| Fichier réel | Rôle | Patch engineDict |
|---|---|---|
| `geometry/PN_Topland.stl`, `PN_Crevice.stl` | Piston (couronne, crevice) | `piston` (movingWall) |
| `geometry/LN_Wall.stl` | Chemise | `liner` (scaling) |
| `geometry/HD_Wall.stl` | Culasse plate (bougie non modélisée v0) | `cylinderhead` (staticWall) |
| `geometry/IV_1_Steam.stl` … `IV_2_Steam.stl` | Tiges de soupapes admission | `iv1_stem`, `iv2_stem` |
| `geometry/IV_?_{Angle,Bottom,Bottom_Side,Seat}.stl` | Têtes de soupapes admission | `iv1_head`, `iv2_head` (movingWall) |
| `geometry/EV_?_{Steam,Angle,Bottom,Bottom_Side,Seat}.stl` | Soupapes échappement | `ev1_stem/ev1_head`, `ev2_stem/ev2_head` |
| `geometry/IP_Wall.stl`, `IP_Seat_1.stl`, `IP_Seat_2.stl` | Conduit admission (mur commun + 2 sièges) | `intakePort` (staticWall) |
| `geometry/IP_Inlet.stl` | Bride admission | `inlet` (groupe inlet) |
| `geometry/EP_Wall.stl`, `EP_Seat_1.stl`, `EP_Seat_2.stl` | Conduit échappement | `exhaustPort` (staticWall) |
| `geometry/EP_Outlet.stl` | Bride échappement | `outlet` (groupe outlet) |
| `geometry/supportSurfaces/IV_?_Cut.stl`, `EV_?_Cut.stl` | Bouchons volumes morts | `supportSurfaces` des blocs `*_head` |

### 18.2 Journal des modifications


> Toute modification du cas doit être consignée ici : fichier modifié, nature
> du changement, motif, date.

| Date | Fichier(s) | Modification | Motif |
|---|---|---|---|
| 2026-10-22 | — | Copie intégrale de `tutorials/tcc3` → `tutorials/PIengine_v0` | Point de départ du nouveau cas |
| 2026-10-22 | `README.md` | Création : documentation d'étude (ce fichier) | Traçabilité des choix et des questions |
| 2026-10-22 | `README.md` (§1, §17) | Découpage CAO finalisé : soupapes à 5 fichiers, conduits à 4 fichiers (`Inlet/Outlet` + `Wall` commun + `Seat_1`/`Seat_2` à 1 tranche) — 36 STL au total | Choix utilisateur |
| 2026-10-22 | `constant/geometry/`, `constant/geometry/supportSurfaces/` | Création des dossiers ; déplacement des 36 STL ; `*Cut*` → `supportSurfaces` ; suppression de `geometry.tar.gz` (géométrie tcc3 obsolète) | Rangement de la géométrie déposée à plat dans `constant/` |
| 2026-10-22 | `Allmesh` | Suppression de la ligne `tar -xzf` (STL livrés en clair) | Adaptation au dépôt direct des STL |
| 2026-10-22 | `constant/engineDict.coarse` | Réécriture complète, adaptée aux 4 soupapes et à la géométrie réelle (alésage R=0.05, culasse plate z=-0.005, soupapes CAO fermées) ; commentaires détaillés de chaque paramètre ; valeurs `TODO-USER` pour la cinématique | Configuration du nouveau moteur |
| 2026-10-22 | `constant/engineDict.fine` | Généré depuis `.coarse` : `cellSize 0.008`, couches limites activées (`addLayers true`, `nSurfaceLayers 1`) | Variante maillage fin |
| 2026-10-22 | `constant/dynamicMeshDict` | Bloc `valves` : 4 entrées (`intakeValve1/2`, `exhaustValve1/2`) ; `slidingPatches` regex `.*Steam.*` ; commentaires | Mover des 4 soupapes (références croisées engineDict) |
| 2026-10-22 | `constant/zonesGenerator` | Sans `sparkPlug` ; `chamber` héritée d'engineDict ; boîte échappement adaptée aux mesures STL ; commentaires | Cohérence avec la nouvelle culasse |
| 2026-10-22 | `README.md` | §18.1 table de correspondance noms réels ; §20 glossaire des paramètres | Documentation des dicts commentés |
| 2026-10-22 | `constant/engineDict.{coarse,fine}` | Décisions utilisateur : course/bielle gardées (0,086/0,231 m), CAO piston = PMH, `clearance 0.017` (mesuré : 17 mm), `minLift` validé 0,5 mm | Données moteur fournies |
| 2026-10-22 | `constant/engineData/` | Lois de levée : CSV de tcc3 conservés (déjà présents par la copie) — IV1=IV2 partagent `intakeLift.txt`, EV1=EV2 partagent `exhaustLift.txt` | Décision utilisateur |
| 2026-10-22 | `0.orig/p` | Pressions aux bornes CONSTANTES (inlet 0,95 bar / outlet 1 bar, `uniformTotalPressure` sans table CSV) ; état initial estimé : 1 bar partout | Décision utilisateur |
| 2026-10-22 | `0.orig/T` | État initial estimé (motored froid) : 300 K partout sauf échappement 350 K ; parois 300 K | Estimation à affiner |
| 2026-10-22 | `system/controlDict` | `rpm 2500` | Décision utilisateur |
| 2026-10-22 | `engineDict.{coarse,fine}` | `insidePoints` repositionnés : conduits (0 ±0.045 0.03) — points initiaux (±0.031) trop proches des puits de soupapes (risque de rib central entre sièges) ; point chambre inchangé (0 0 -0.013) | Robustesse snappy (§21.3) |
| 2026-10-22 | `README.md` | Ajout §21 : notes de validation (clearance, chemise scaling, insidePoints, affrontement soupape-siège) | Consignation des remarques de la session |
| 2026-11-03 | `constant/geometry/HD_Wall.stl` | **Correction d'analyse** : le « trou central » annoncé n'existait PAS — le fichier n'a que les 4 trous de soupapes (l'aire des triangles = disque − 4 puits, exactement ; r=13,8 mm = bord interne des puits, pas d'un 5ᵉ trou). Fichier utilisateur validé ✓ | Erreur d'interprétation du raster (cf. §21.9) |
| 2026-11-03 | `IP_Wall.stl`, `EP_Wall.stl`, `supportSurfaces/*_Cut.stl` | **ERRATUM / ANNULATIONS** : (1) la « fente des conduits » n'existait pas — les 88 arêtes de bord des murs à z=−0.003 sont toutes à r=12,5 des axes de soupape = **bord supérieur exact des sièges** (`IP_Seat` : cône r 12,5@−0.003 → 14,5@−0.005) qui réalisent la jonction mur→culasse (correction utilisateur) → jupe supprimée, murs restaurés ; (2) l'élévation des colliers Cut (+1 mm) a été ANNULÉE : leurs bords inférieurs se posent à l'origine pile sur les bords des trous de la culasse (r=14,5, z=−0.005) — l'élever créait une fuite de 1 mm | Erreurs d'analyse corrigées (cf. §21.12bis) |
| 2026-11-03 | `IP_Wall.stl`, `EP_Wall.stl` | **FUITE PRINCIPALE CORRIGÉE** : les murs de conduits s'arrêtaient à z=−0.003, laissant une **fente ouverte de 2 mm** entre leur fond et le plan de culasse (z=−0.005) — 88 arêtes de bord par mur à z=−0.003 (ouverture 65×25 mm). Cette fente reliait les conduits admission↔échappement↔extérieur au-dessus de la plaque → tout le domaine connecté à l'extérieur → snappy ne supprimait rien. Correctif : jupes verticales de 2 mm générées par script sur chaque arête du bord inférieur (176 tris/mur), murs désormais posés sur la culasse (z=−0.005) | Symptôme « cellules extérieures non supprimées » (cf. §21.12) |
| 2026-11-03 | STL + `engineDict.{coarse,fine}` | **Soupapes échappement décalées** : pièces EV_1/EV_2 (Steam, Angle, Seat, Bottom, Bottom_Side) translatées de **−1 mm** en z (CAO = 1 mm sous la position fermée, style tcc3 EV 9.5e-4) ; `initialPosition 0.001` dans `ev1_stem`/`ev2_stem` (à t=0, EV remonte de 1 mm → fermée). `*_Cut` translatés **+1 mm** (décoplanarisation plan culasse) — les collaires restent ancrés à la culasse, comme tcc3 | Symptôme persistant « cellules extérieures non supprimées » (cf. §21.11) |
| 2026-11-03 | `Allmesh` | **Mode débogage** : `DEBUG_MESH=1 ./Allmesh` = un seul maillage à l'instant 0 (saute predictRemeshInstants, écrit 0 dans meshTimes) | Débogage géométrie/maillage (cf. §21.10) |
| 2026-11-03 | `constant/geometry/supportSurfaces/*_Cut.stl` | Nouveaux ValveCut validés : colliers cylindriques r=14,5 mm, z[-0.005,0], enjambant sièges et traversant les parois (2 mm de pénétration) — conforme au design tcc3 ✓ | Correction CAO utilisateur |
| 2026-11-03 | `Allmesh` | `generateMesh` introuvable via PATH (script du dépôt, pas un utilitaire OpenFOAM) → Allmesh le localise désormais via `../../bin/generateMesh` avec repli sur le PATH | Erreur "generateMesh: not found" au premier maillage (cf. §21.8) |
| 2026-11-03 | `constant/geometry/` + `.gitignore` | **Incident corrigé** : les 36 STL avaient disparu du dépôt (le `.gitignore` global contient `*.stl` ; le `git add` d'un déplacement les a silencieusement exclus et enregistré leur suppression). Restaurés avec `git add -f` + exceptions `!tutorials/PIengine_v0/constant/geometry/**` dans `.gitignore` | Piège git (§21.7) |
| 2026-11-03 | `engineDict.{coarse,fine}` | **Bugfix** : références d'héritage relatives corrigées — `$../piston/...`, `$../../iv1_stem/...`, `$../../ev1_stem/...` → `$../../../...` (il faut 3 niveaux depuis un bloc `motion` : motion→geometryMotion→patch→patches) ; détecté par le FATAL IO ERROR ligne 216 au premier Allmesh | Erreur de syntaxe dict (cf. §21.6) |

## 19. TODO avant premier maillage

- [x] Recevoir et vérifier les STL (réceptionnés, bounding-boxes mesurées ; `surfaceCheck` complet à faire côté OpenFOAM)
- [x] Adapter `engineDict.coarse` (et `.fine`) — 4 soupapes, insidePoints, raffinements
- [x] Adapter `dynamicMeshDict` (4 valves, slidingPatches regex)
- [x] Adapter `zonesGenerator` (chambre, ports, cylinderHead/pistonBowl)
- [x] CSV `engineData` : levées tcc3 conservées ; pressions passées en constantes dans `0.orig/p`
- [ ] Mettre à jour `Allmesh` si la géométrie n'est plus livrée en tar.gz
- [ ] `./Allclean && ./Allmesh`
- [ ] `checkMesh` sur `constant/meshes/0/polyMesh` **avant** la boucle complète
- [ ] Ajuster `cellSize`/raffinements si nécessaire, répéter
- [ ] `./Allrun`

## 20. Glossaire des paramètres des dictionnaires (réponses aux questions)

Chaque paramètre ci-dessous est commenté en place dans les dictionnaires.
Ce glossaire sert de référence rapide.

### engineDict (patches/*/geometryMotion)

| Paramètre | Sens | Où le trouver / comment le fournir |
|---|---|---|
| `axis (0 0 1)` | Axe du mouvement (moteur ou soupape) | Vertical ici ; l'adapter si soupapes inclinées |
| `initialPosition` | Position de référence de la CAO (levée pour une soupape, hauteur pour le piston). `déplacement(t) = valeur(t) − initialPosition` ; défaut = 0 | CAO PIengine_v0 : soupapes FERMÉES et piston à sa position CAO → 0 partout (vérifier la position piston, voir question Q1) |
| `minLift` | Levée minimale de soupape : la soupape n'est jamais totalement au contact dans le maillage ; déclenche les remaillages d'ouverture/fermeture (`forcedRemeshPositions`) | Choix maillage (tcc3 : 0,5 mm) ; doit être ≥ clarance CAO fermée |
| `motion { type crankConnectingRodMotion; conRodLength; stroke; }` | Cinématique piston : longueur de bielle et course en m | **Données moteur à fournir (TODO-USER)** |
| `motion { type repeat; period 720; value { type table; file ...; } }` | Loi de levée : table CSV CAD → lift, répétée chaque cycle | CSV dans `constant/engineData/` (TODO-USER : lois propres à PIengine) |
| `remeshControl { absTolerance; relTolerance; forcedRemeshPositions; clearance; }` | Quand remailler : tolérances sur le déplacement depuis le dernier maillage ; instants forcés aux fermetures ; clarance morte au PMH | Tolérances transposables de tcc3 ; `clearance` = donnée moteur (TODO-USER) |

### engineDict (meshControl)

| Paramètre | Sens | Où le trouver |
|---|---|---|
| `cellSize` | Taille de base du maillage (avant raffinements, chaque niveau divisant par 2) | ~alésage/6 en grossier |
| `insidePoints ((x y z) ...)` | UN POINT PAR VOLUME FLUIDE ; doit rester dans le fluide à tous les instants | Centre chambre + centre de chaque conduit (vérifier après course connue) |
| `surfaceRefinement N` | Niveau de raffinement d'une surface (2^N division) | Sièges/fentes > parois > conduits |
| `distanceRefinement ((d1 l1) (d2 l2))` | Raffinement par distance à la surface (bandes concentriques) | Pour la chemise et la fente de levée |
| `dynamicRefinement true; numberOfCells 3;` | À chaque remaillage, taille recalculée pour poser N cellules à travers la fente soupape-siège | Sur les `*_Seat` des soupapes |
| `regionRefinement { type cylinder; point1; point2; radius; level; }` | Raffinement volumique analytique (cylindre, cône tronqué...) ; fenêtres `begin/end/repeat` pour activation par CAD | Zone chambre héritée par zonesGenerator |
| `layerControl` | Couches limites (prismes) : nombre, épaisseurs, expansion | Off en v0 coarse, on en fine |

### engineDict (patches/*/meshControl/patchInfo)

| Paramètre | Sens |
|---|---|
| `inGroups staticWall;` | Paroi fixe (noSlip) — défaut global si non listé |
| `inGroups movingWall;` | Paroi mobile (movingWallVelocity : vitesse locale auto) |
| `patchType patch; inGroups inlet/outlet;` | Vraie frontière : conditions inlet/outlet des 0.orig |

### dynamicMeshDict

| Paramètre | Sens |
|---|---|
| `slidingPatches` | Patches où le maillage glisse (chemise, tiges) — regex sur les noms DE FICHIERS (`.*Steam.*` ici) |
| `frozenZones` / `movingZones` | PointZones ancrées (cylasse) / suivant le piston (pistonBowl), définies dans zonesGenerator |
| `${engineDict!...}` | Référence croisée : valeur lue dans engineDict (source unique) |
| `topoChanger { times; repeat; timeDelta; }` | Instants de remaillage (meshTimes), périodicité du cycle, tolérance d'appariement |

### zonesGenerator

| Paramètre | Sens |
|---|---|
| `zoneType cell;` | Sélectionne des cellules (cellZone) — pour initialisation zonale et post-traitement |
| `type cylinder/box; point1/point2/radius/box` | Formes analytiques ; coordonnées mesurées sur les STL PIengine_v0 |
| `type union/difference` | Combinaisons booléennes de zones (ordre de déclaration important) |
| `$engineDict!...` | Héritage d'une définition d'engineDict (raffinement ↔ zone cohérents) |
| `cylinderHeadPoints` / `pistonBowlPoints` | PointZones du mover (frozen/moving) recalculées après remaillage (`moveUpdate`) |
| `periodic { begin; end; deactivate; repeat; }` | Activation de la zone pistonBowl dans une fenêtre CAD autour du PMH |

## 21. Notes de validation et remarques importantes

### 21.1 Rôle exact de `clearance` (≠ position du piston)

La position du piston vient uniquement de la cinématique
(`stroke`/`conRodLength`/`initialPosition`). `clearance` **ne positionne rien** :
il est utilisé uniquement par `predictRemeshInstants`
(`applications/utilities/predictRemeshInstants/remesh/patchRemesh.C:80`) : le
déplacement relatif du piston est normalisé par `(déplacement précédent +
clearance)`. Sans cela, près du PMH (chambre minuscule), un petit déplacement
absolu représenterait un changement relatif énorme et forcerait des remaillages
à chaque pas. Valeur PIengine_v0 : **0,017 m** (mesuré CAO au PMH : couronne
z = −0,022, culasse z = −0,005).

### 21.2 Convention de hauteur de la chemise (scalingMotion)

La CAO de la chemise représente sa position **la plus courte (PMH)** ; le mover
`scalingMotion` l'**étire** (facteur > 1) à mesure que le piston descend, pour
qu'elle couvre toujours culasse → piston. Vérifié sur tcc3 : sa chemise fait
exactement sa clarance PMH (9,5 mm). PIengine_v0 : chemise 20 mm au PMH
(17 mm de chambre + 3 mmm dans la zone crevice) → **conforme à la convention**.
NE PAS modéliser la chemise « haute au PMB » : elle serait comprimée au PMH.

### 21.3 `insidePoints` — position retenue et raisonnement

Un point par volume fluide déconnectable, TOUJOURS dans le fluide (les points
sont recopiés dans le snappyHexMeshDict de chaque instant) :

| Volume | Point | Justification |
|---|---|---|
| Chambre | `(0 0 -0.013)` | 8 mm sous la culasse, 9 mm au-dessus de la couronne au PMH ; la chambre ne fait que grandir ensuite (le seul point dépendant du piston) |
| IntakePort | `(0 -0.045 0.03)` | zone large du conduit, mi-hauteur, loin des puits de soupapes ; conduit immobile → toujours valide |
| ExhaustPort | `(0 0.045 0.03)` | idem côté échappement |

⚠️ Première version `(±0.031 0.02)` : écartée car trop proche de la bande des
sièges (y ∈ [±0.0345, ±0.0055]) — si un rib central sépare les deux puits dans
le port, le point serait dans le solide. À **valider au premier Allmesh**
(snappy garde le volume contenant chaque point ; vérifier que chamber + ports
sont bien les 3 volumes attendus).

### 21.4 Affrontement soupape/siège à la fermeture — point d'attention CAO

Mesure CAO (soupapes fermées) : faces `IV/EV_?_Bottom` à z = −0,0059 ; plan des
sièges (`IP/EP_Seat_?`) à z ∈ [−0,0050, −0,0030] → la face de soupape **pénètre
de 0,9 mm sous le plan d'appui du siège**. Interprétations possibles :

- soit la position CAO fermée correspond effectivement à un léger enfoncement
  (certaines CAO coupent la soupape au plan du siège) : acceptable, le maillage
  « voit » une soupape affrontée, mais **à vérifier en CAO** ;
- soit c'est une interférence à corriger (remonter la soupape de 0,9 mm ou
  ajuster `initialPosition` — cf. §13 : `déplacement(t) = lift(t) −
  initialPosition`).

Si correction choisie via `initialPosition` : mettre `initialPosition 0.0009`
(déclarer « CAO = levée 0,9 mm ») — le reste suit automatiquement.

### 21.5 Divers

- Coquille de nomenclature conservée : `Steam` au lieu de `Stem` (fichiers et
  dicts cohérents entre eux, regex du mover `.*Steam.*`).
- Les CSV `intakePressure.txt`/`exhaustPressure.txt` ne sont plus référencés
  (pressions constantes dans `0.orig/p`) mais restés dans `engineData/` pour
  un éventuel retour aux tables.
- `chamberPressure.txt` (donnée expérimentale tcc3) : **inutilisable pour
  PIengine_v0** (moteur différent) — à remplacer par des essais ou à supprimer
  des scripts de comparaison (`plot_scripts/chamberPressure.py`).
| (à venir) | `constant/engineDict` | Passage à 4 soupapes (blocs `iv2_*`, `ev2_*`, sièges à 1 tranche, `sparkplug` supprimé, conduits à 4 fichiers) | Adaptation à la nouvelle géométrie |
| (à venir) | `constant/dynamicMeshDict` | Bloc `valves` : `intakeValve2`, `exhaustValve2` | Mover des 2 nouvelles soupapes |
| (à venir) | `constant/engineData/` | CSV de levée ×4 + pressions | Lois propres à PIengine_v0 |
| (à venir) | `constant/zonesGenerator` | Sans `sparkPlug`, ports adaptés | Cohérence avec la nouvelle culasse |
| (à venir) | `constant/geometry/` | Remplacement des STL (§17) | Nouvelle CAO 4 soupapes |

### 21.6 Piège de syntaxe : références d'héritage relatives (`$../...`)

Le nombre de `../` se compte depuis le bloc **où vit la référence**, pas depuis
le bloc qu'on écrit. Depuis l'intérieur d'un bloc `motion`
(`patches/<p>/geometryMotion/motion`), il faut **3** niveaux pour remonter à
`patches` :

```
motion { $../../../piston/geometryMotion/motion; }   // correct
motion { $../piston/geometryMotion/motion; }         // FATAL IO ERROR
```

Règle : écrire la référence, puis compter les accolades ouvertes entre le bloc
courant et `patches`. Les références absolues (`${engineDict!...}` dans
dynamicMeshDict) ne sont pas soumises à ce piège.

### 21.7 Piège git : le `.gitignore` exclut `*.stl`

Le `.gitignore` du dépôt OpenFOAM contient `*.stl` (c'est pourquoi tcc3 livre
sa géométrie en `geometry.tar.gz`). Conséquences :

- un `git add` de STL est **silencieusement ignoré** (aucun message) ;
- un upload via l'interface web GitHub, lui, les ajoute ;
- déplacer des STL suivis par git vers un chemin ignoré = commit de suppression.

Correctifs appliqués :
1. exceptions dans `.gitignore` :
   `!tutorials/PIengine_v0/constant/geometry/*.stl` et
   `!tutorials/PIengine_v0/constant/geometry/supportSurfaces/*.stl` ;
2. réintégration forcée (`git add -f`), commit 72a61b9.

Règle : après toute manipulation de fichiers STL dans le dépôt, vérifier avec
`git ls-files tutorials/PIengine_v0/constant/geometry | wc -l` (attendu : 36).

### 21.8 `generateMesh` : script du dépôt, pas un utilitaire OpenFOAM

`generateMesh` vit dans `bin/` du dépôt (il lance blockMesh+snappyHexMesh,
localement ou via SGE/SLURM). Il n'est PAS installé par OpenFOAM : si `bin/`
n'est pas dans le PATH, Allmesh échoue avec « generateMesh: not found ».

Correctifs :
- immédiat : `export PATH="$PATH:<repo>/bin"`, puis relancer la boucle à la main ;
- durable (appliqué dans Allmesh) : le script cherche
  `$(dirname $0)/../../bin/generateMesh` et retombe sur le PATH.

Vérification : `which generateMesh` doit rendre un chemin avant de lancer.

### 21.9 Erratum : le HD_Wall n'a jamais eu de trou central

L'analyse raster du §précédent avait été mal interprétée : les 4 vides visibles
sont les puits de soupapes, le centre (0,0) est couvert. Preuve arithmétique :
aire des triangles (0,006862 m²) = aire du disque r=0,055 (0,009503) −
4 × π × 0,0145² (0,002642) — aucun 5ᵉ trou. Le rayon minimal des sommets
(0,0138 m) est le bord interne des puits (centre à 28,3 mm, rayon 14,5 mm),
pas le rayon d'un trou central. Leçon : valider une « fuite » par le calcul
d'aire avant de conclure.

### 21.10 Mode débogage d'Allmesh (maillage unique)

`DEBUG_MESH=1 ./Allmesh` :
- saute `predictRemeshInstants` et écrit `0` dans `constant/meshTimes` ;
- `moveSurfaces`/`engineMeshConfig` traitent uniquement l'instant 0 (les temps
  venant de `meshTimes` via l'include d'`engineDict`) ;
- la boucle génère un seul maillage : `constant/meshes/0/polyMesh` ;
- contrôles : `checkMesh constant/meshes/0/polyMesh` + ParaView (vérifier que
  seuls chambre + conduits sont maillés, que l'extérieur est supprimé).

Sortie du mode : `./Allclean` puis `./Allmesh` normal. Les utilitaires
`moveSurfaces`/`engineMeshConfig` acceptent aussi `-times "(0)"` manuellement.

### 21.11 Décalage des soupapes d'échappement (style tcc3)

Pour reproduire la convention tcc3 (EV : CAO 0,95 mm sous la fermée,
`initialPosition 9.5e-4`), les pièces des soupapes d'échappement ont été
translatées de −1 mm en z dans les STL, et `initialPosition 0.001` déclaré
dans `ev1_stem`/`ev2_stem` :

```
déplacement(t) = lift(t) − 0.001
```

À t=0 : lift=0 → déplacement = −0,001 → la soupape REMONTE de 1 mm dans
`moveSurfaces` → position fermée. Le maillage de t=0 est inchangé ; seule la
position de RÉFÉRENCE CAO diffère (comme tcc3). Les admissions restent
fermées en CAO (`initialPosition 0`).

Commandes équivalentes OpenFOAM-14 (syntaxe : transformation = argument
positionnel, PAS une option) :

```bash
surfaceTransformPoints "translate=(0 0 -0.001)" EV_1_Bottom.stl out.stl
```

NOTE : les collaires `*_Cut` ne suivent PAS les soupapes vers le bas — ils
restent positionnés par rapport à la culasse (tcc3 : ses deux collaires à la
même hauteur au-dessus du plan de culasse alors que sa CAO EV est décalée).

### 21.12 La fente des conduits : la vraie cause du « cellules extérieures »

Diagnostic par les **arêtes de bord** (edges appartenant à 1 seul triangle) :
IP_Wall et EP_Wall avaient chacun 88 arêtes de bord à z = −0,003 — le fond
des conduits était **ouvert**, 2 mm au-dessus du plan de culasse (−0,005).
Rien n'occupant l'espace z ∈ [−0,005 ; −0,003] (hors anneaux de siège), cette
fente reliait : conduit admission ↔ zone morte au-dessus de la plaque ↔
conduit échappement ↔ extérieur (au-delà du bord de la plaque, r > 0,055).
Tous les volumes étant connectés à l'extérieur, snappyHexMesh gardait tout.

Correctif : extrusion verticale (jupe) de chaque arête du bord inférieur
jusqu'au plan de culasse (script Python, 176 triangles par mur). Le bord des
murs est désormais posé sur la plaque (z = −0,005), comme le haut de la
chemise. Règle générale : **toute pièce doit finir SUR une autre pièce**,
jamais en laissant un intervalle. (Les brides d'entrée/sortie et les trous
de tiges du plafond des conduits sont, eux, conformes au design tcc3.)

### 21.12bis Erratum : sièges = pièces de jonction ; colliers posés sur la culasse

Deux corrections d'analyse (mesures, correction utilisateur) :

1. **Les murs de conduits n'ont jamais eu de fente ouverte** : leurs 88 arêtes
   de bord à z=−0.003 sont toutes à r=12,5 mm des axes de soupape — elles
   s'affrontent **exactement** au bord supérieur des anneaux de siège
   (`IP/EP_Seat_*` : cône de r 12,5 à z=−0.003 vers r 14,5 à z=−0.005).
   Les sièges sont les **pièces de jonction** mur → plan de culasse. La jupe
   ajoutée à tord (§21.12) a été supprimée, murs restaurés à l'identique.
2. **Les colliers ValveCut étaient correctement posés à l'origine** : leur
   bord inférieur (r=14,5, z=−0.005) coïncide avec les bords des trous de
   culasse. L'élévation de +1 mm (analogie tcc3) a été **annulée** : elle
   ouvrait une fuite de 1 mm. Notre culasse étant de épaisseur nulle
   (plaque à z=−0.005), le design tcc3 (colliers surélevés au-dessus d'une
   culasse épaisse) ne transposait pas.

Leçon : chaque « fuite » doit être validée par affrontement mesuré
(arêtes mises en regard des bords des pièces voisines) avant correction.
