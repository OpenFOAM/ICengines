# PIengine_v1 — Cas moteur 4 soupapes AVEC combustion H₂ (XiFluid)

> **Statut : démarrage.** Copie de `PIengine_v0` (cas motored validé), en cours
> d'évolution vers la **combustion prémélangée H₂-air** via le solveur
> **`XiFluid`** d'OpenFOAM-14. Le spray (eau liquide) est **mis de côté**
> (prévu en v2). Ce README documente le choix du solveur, le plan de
> transformation et les décisions à prendre.
>
> Pour l'historique complet du débogage (géométrie, fuite, sur-raffinement,
> crevice, planning de remaillage), voir le README de `PIengine_v0` (§22).

## Sommaire

1. [Pourquoi XiFluid](#1-pourquoi-xifluid)
2. [Le modèle b-Xi de Weller](#2-le-modèle-b-xi-de-weller)
3. [Compatibilité avec AATE](#3-compatibilité-avec-aate)
4. [Le modèle à copier : moriyoshiHomogeneous](#4-le-modèle-à-copier-moriyoshihomogeneous)
5. [Checklist de transformation v0 → v1](#5-checklist-de-transformation-v0--v1)
6. [Décisions à prendre (spécifications moteur)](#6-décisions-à-prendre-spécifications-moteur)
7. [Points d'intégration critiques](#7-points-dintégration-critiques)
8. [Perspectives v2 : spray (eau liquide)](#8-perspectives-v2-spray-eau-liquide)
9. [Journal des modifications](#9-journal-des-modifications)

---

## 1. Pourquoi XiFluid

`XiFluid` est le module Foundation 12+ qui hérite de la famille
`XiFoam`/`engineFoam` — **conçu à l'origine pour les moteurs à allumage
commandé**. Il résout la combustion **prémélangée** par le modèle **b-Xi de
Weller** (variable de progrès + plissement de flamme), exactement le cadre
voulu pour H₂-air :

- architecture Foundation 12+ : module chargé par `foamRun`
  (`solver XiFluid;` dans `controlDict`), remplaçant `fluid` sans toucher au
  reste ;
- héritage moteur : gestion du temps moteur (CAD), allumage par bougie dans
  les tutoriels historiques ;
- combustion prémélangée turbulente : le cœur du modèle b-Xi.

Alternatives écartées : la combustion **diffusionnelle** (pas le régime H₂
visé), les modèles de flamme épaisse/laminarFlameSpeed (moins adaptés au
moteur), et le portage vers ESI `reactingFoam` (incompatible avec
l'écosystème AATE, voir §3).

## 2. Le modèle b-Xi de Weller

Le modèle sépare la cinétique fine de la propagation turbulente :

| Champ | Sens | Initialisation typique |
|---|---|---|
| `b` | variable de progrès régressée (b=1 gaz brûlés, b=0 gaz frais) | 1 dans la chambre (gaz frais) |
| `Xi` | facteur de plissement de flamme (rapport surface turbulente/laminaire) | calculé par le modèle, init ~1 |
| `Su` | vitesse de flamme laminaire — modèle `laminarFlameSpeed` (constante, corrélation, ou `coded`) | corrélation H₂ (voir §5.3) |
| `ft` | fraction massique de carburant total (prémélangé partiel) | selon la richesse remplie |

Le taux de réaction s'écrit ~ `rho·Su·Xi/Δ` consommant `b` ; la flamme naît
aux **sites d'allumage** (spark) puis se propage. Pour l'H₂ : Su laminaire
élevée (~2,5 m/s à stoich, ambiante — contre ~0,4 pour l'essence), d'où
l'intérêt d'une corrélation adaptée plutôt que d'une constante générique.

## 3. Compatibilité avec AATE

| Élément AATE | Impact du passage à XiFluid |
|---|---|
| `mover` `multiValveEngine` (piston, soupapes, chemise) | **inchangé** — agit au niveau maissage (`dynamicMeshDict`), indépendant du solveur |
| `topoChanger` `meshToMesh` (remaillages) | **inchangé** — idem |
| Pipeline `engineDict`/`Allmesh` (engineMeshConfig, moveSurfaces...) | **inchangé** — les STL et patches ne dépendent pas du solveur |
| Champs `0.orig` | à compléter : `b`, `Xi`, `ft` + adaptation p/T/composition |
| `fvConstraints` | `limitTemperature` (301–2800 K) déjà en place ✓ ; resserrer `limitPressure` selon la combustion |

⚠️ Les **références croisées** `${engineDict!...}` du `dynamicMeshDict`
restent valides. Aucune modification C++ nécessaire : XiFluid est un module
standard d'OpenFOAM-14.

## 4. Le modèle à copier : moriyoshiHomogeneous

Le tutoriel historique de la famille XiFoam est un **cas moteur
prémélangé avec allumage par bougie, en temps moteur (CAD)** — le blueprint
idéal. Vérifier sa présence dans l'installation :

```bash
find /opt/openfoam14/tutorials -ipath "*XiFluid*" -type d
ls /opt/openfoam14/tutorials/combustion/XiFluid/ 2>/dev/null
```

S'il est présent (`moriyoshiHomogeneous` ou équivalent), ses fichiers
fournissent le format exact pour `combustionProperties`,
`physicalProperties` réactif, champs `b`/`Xi`/`ft`, et l'**ignition**
(sites, durée, force). Adaptation : géométrie/cinématique → déjà la nôtre ;
mélange essence → H₂ ; échelle → la nôtre.

## 5. Checklist de transformation v0 → v1

1. **`system/controlDict`** : `solver fluid` → `solver XiFluid` (vérifier
   `libs` si requis) ;
2. **`constant/physicalProperties`** : mélange réactif **H₂/O₂/N₂ (+H₂O)**
   — thermophysique compressible `psi`, réaction 1-step
   `2H2 + O2 -> 2H2O` (+N2 inerte), définition du « fuel » pour b-Xi ;
3. **`constant/combustionProperties`** (nouveau) :
   - `combustionModel` : infinitelyFastChemistry ;
   - `laminarFlameSpeed` : corrélation H₂ (constante pour v0, ou `coded`
     sur corrélation type Gulders/éq. adaptée H₂) ;
   - **`ignition`** : sites d'allumage (position = axe chambre, rayon
     ~5-8 mm, durée en secondes ↔ CAD, force) ;
4. **`0.orig/`** : ajouter `b` (=1, gaz frais partout), `Xi` (=1),
   `ft` (fraction H₂ selon richesse) ; adapter `p`, `T`, `tracer`
   éventuellement retiré ;
5. **`constant/zonesGenerator`** : réintroduire une **cellZone d'allumage**
   au centre de la chambre (cylindre/sphère, cf. tcc3 `sparkPlug` — l'allumage
   est une ZONE, pas une pièce CAO : la culasse plate reste telle quelle) ;
6. **`constant/engineDict`** : réactiver `regionRefinement
   chamber_combustion` (raffinement temporel autour de l'allumage — le bloc
   commenté existe déjà dans la copie) ;
7. **`system/fvSchemes`/`fvSolution`** : schémas et solveurs pour `b`/`Xi`
   (copier du tutoriel moriyoshi) ;
8. **`fvConstraints`** : resserrer `limitPressure` (la compression avec
   dégagement de chaleur H₂ est rapide).

## 6. Décisions à prendre (spécifications moteur)

| # | Paramètre | Options | Décision |
|---|---|---|---|
| 1 | **Richesse φ H₂-air** | stoich (φ=1) / appauvri (φ=0,5–0,7, typique H₂) | ⬜ |
| 2 | **Remplissage** | prémélangé homogène à t=0 (v1 recommandé) / injection pendant l'admission (→ nécessite injecteur, proche du spray v2) | ⬜ |
| 3 | **Avance à l'allumage** | CAD de spark (ex. ~340° = −20° avant PMH compression) | ⬜ |
| 4 | **Énergie d'allumage** | durée du spark (CAD ou s), intensité — ou valeurs tutoriel adaptées | ⬜ |
| 5 | **Chimie H₂** | 1-step (robuste, v1) / schéma détaillé (v2) | ⬜ |
| 6 | **Régime/point** | garder 2500 tr/min, PMH CAO (géométrie v0 inchangée) ? | ⬜ |

## 7. Points d'intégration critiques

1. **Stabilité numérique H₂** : Su élevée + maillage en mouvement → le pas
   de temps (déjà `adjustTimeStep`/`maxCo 10`) s'adaptera ; surveiller que le
   `deltaT` ne s'effondre pas au passage de flamme ; `limitTemperature`
   protège déjà ;
2. **Flamme vs remaillages meshToMesh** : les champs `b`/`Xi`/`ft` sont
   interpolés comme les autres par le topoChanger — vérifier la conservation
   de la variable de progrès au premier remaillage post-allumage ;
3. **Allumage vs zone** : le site d'allumage doit être **dans une cellule
   fine** (le raffinement `chamber_combustion` autour du CAD d'allumage y
   pourvoit) ;
4. **H₂ dans les conduits à t=0** (si prémélangé partout) : la composition
   `ft` initiale doit distinguer chambre (mélange) et conduits (air seul),
   via les zones `chamber`/`intakePort`/`exhaustPort` déjà définies dans le
   `zonesGenerator` — même mécanique que les `internalField` zonaux actuels.

## 8. Perspectives v2 : spray (eau liquide)

Mise de côté volontairement. État des connaissances (2026-11) :

- l'équipe AATE/Wärtsilä a déjà fait tourner du spray Lagrangien Foundation
  (log `pyaate/tests/test_data/openfoam_data/phase_change_log.txt` :
  `Solving 3-D cloud sprayCloud`, `coneInjection`, function objects maison
  `vaporPenetration`/`getViscosity`) ;
- OpenFOAM-14 dispose d'un module Lagrangien ; l'équivalent ESI
  (`sprayFoam` v2606) n'est PAS portable sans refonte d'AATE ;
- plan probable (v2) : fvModel cloud (couplage faible) puis solveur maison
  héritant du module combustion+particules (couplage fort) ; point dur :
  relocalisation des parcels aux remaillages meshToMesh.

## 9. Journal des modifications

| Date | Fichier(s) | Modification | Motif |
|---|---|---|---|
| 2026-11-03 | — | Copie intégrale de `PIengine_v0` (validé motored) → `PIengine_v1` | Point de départ du cas combustion |
| 2026-11-03 | `README.md` (ce fichier) | Création : choix XiFluid, modèle b-Xi, checklist, décisions | Feuille de route combustion H₂ |
| 2026-11-03 | `.gitignore` (racine) | Exceptions `*.stl` étendues à `PIengine_v1` (leçon v0 §21.7 : sans ça les STL sont perdus au commit) | Conservation de la géométrie |
