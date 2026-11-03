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

> Toute modification du cas doit être consignée ici : fichier modifié, nature
> du changement, motif, date.

| Date | Fichier(s) | Modification | Motif |
|---|---|---|---|
| 2026-10-22 | — | Copie intégrale de `tutorials/tcc3` → `tutorials/PIengine_v0` | Point de départ du nouveau cas |
| 2026-10-22 | `README.md` | Création : documentation d'étude (ce fichier) | Traçabilité des choix et des questions |
| 2026-10-22 | `README.md` (§1, §17) | Découpage CAO finalisé : soupapes à 5 fichiers, conduits à 4 fichiers (`Inlet/Outlet` + `Wall` commun + `Seat_1`/`Seat_2` à 1 tranche) — 36 STL au total | Choix utilisateur |
| (à venir) | `constant/engineDict` | Passage à 4 soupapes (blocs `iv2_*`, `ev2_*`, sièges à 1 tranche, `sparkplug` supprimé, conduits à 4 fichiers) | Adaptation à la nouvelle géométrie |
| (à venir) | `constant/dynamicMeshDict` | Bloc `valves` : `intakeValve2`, `exhaustValve2` | Mover des 2 nouvelles soupapes |
| (à venir) | `constant/engineData/` | CSV de levée ×4 + pressions | Lois propres à PIengine_v0 |
| (à venir) | `constant/zonesGenerator` | Sans `sparkPlug`, ports adaptés | Cohérence avec la nouvelle culasse |
| (à venir) | `constant/geometry/` | Remplacement des STL (§17) | Nouvelle CAO 4 soupapes |

## 19. TODO avant premier maillage

- [ ] Recevoir et vérifier les STL (`surfaceCheck` : fermeture, orientation, unités m)
- [ ] Adapter `engineDict.coarse` (et `.fine`) — 4 soupapes, insidePoints, raffinements
- [ ] Adapter `dynamicMeshDict` (4 valves, slidingPatches regex)
- [ ] Adapter `zonesGenerator` (chambre, ports, cylinderHead/pistonBowl)
- [ ] Fournir/générer les CSV `engineData` (levées, pressions)
- [ ] Mettre à jour `Allmesh` si la géométrie n'est plus livrée en tar.gz
- [ ] `./Allclean && ./Allmesh`
- [ ] `checkMesh` sur `constant/meshes/0/polyMesh` **avant** la boucle complète
- [ ] Ajuster `cellSize`/raffinements si nécessaire, répéter
- [ ] `./Allrun`
