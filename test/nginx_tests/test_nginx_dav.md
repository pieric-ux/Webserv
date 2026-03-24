# Tests nginx — PUT / POST / DELETE

> Légende : ✅ testé | ⬜ à faire | **?** résultat incertain

---

## Setup

### Structure des fichiers

```
nginx_tests/
├── nginx.conf              ← config active (chargée par nginx)
├── use-config.sh           ← switche la config et recharge nginx
├── setup-idx.sh            ← ajoute ou supprime www/test/index.html
├── run-tests.sh            ← exécute tous les tests, génère output/
├── configs/
│   ├── dav-off_ai-off.conf
│   ├── dav-off_ai-on.conf
│   ├── dav-on_ai-off.conf
│   ├── dav-on_ai-on.conf
│   ├── dav-on_ai-off_cfpp-off.conf
│   └── dav-on_ai-on_cfpp-off.conf
├── output/                 ← 23 fichiers, un par ligne de test
└── www/
    ├── (index.html)        ← optionnel
    └── test/
        └── (index.html)   ← optionnel, géré par setup-idx.sh
```

### Configs disponibles

| Fichier | DAV | autoindex | create_full_put_path |
|---------|-----|-----------|----------------------|
| `configs/dav-off_ai-off.conf` | off | off | — |
| `configs/dav-off_ai-on.conf` | off | on | — |
| `configs/dav-on_ai-off.conf` | on | off | on |
| `configs/dav-on_ai-on.conf` | on | on | on |
| `configs/dav-on_ai-off_cfpp-off.conf` | on | off | off |
| `configs/dav-on_ai-on_cfpp-off.conf` | on | on | off |

### Switcher de config

```bash
# Appliquer une config et recharger nginx
./use-config.sh dav-on_ai-off

# Gérer index.html dans www/test/
./setup-idx.sh plus    # crée www/test/index.html
./setup-idx.sh minus   # supprime www/test/index.html

# Lancer tous les tests
./run-tests.sh
```

### Workflow par colonne

| Colonne | Commandes |
|---------|-----------|
| DAV- · AI- · idx- | `./use-config.sh dav-off_ai-off` + `./setup-idx.sh minus` |
| DAV- · AI- · idx+ | `./use-config.sh dav-off_ai-off` + `./setup-idx.sh plus` |
| DAV- · AI+ · idx- | `./use-config.sh dav-off_ai-on` + `./setup-idx.sh minus` |
| DAV- · AI+ · idx+ | `./use-config.sh dav-off_ai-on` + `./setup-idx.sh plus` |
| DAV+ · AI- · idx- | `./use-config.sh dav-on_ai-off` + `./setup-idx.sh minus` |
| DAV+ · AI- · idx+ | `./use-config.sh dav-on_ai-off` + `./setup-idx.sh plus` |
| DAV+ · AI+ · idx- | `./use-config.sh dav-on_ai-on` + `./setup-idx.sh minus` |
| DAV+ · AI+ · idx+ | `./use-config.sh dav-on_ai-on` + `./setup-idx.sh plus` |

---

## POST

| URL | DAV-<br>AI-<br>idx- | DAV-<br>AI-<br>idx+ | DAV-<br>AI+<br>idx- | DAV-<br>AI+<br>idx+ | DAV+<br>AI-<br>idx- | DAV+<br>AI-<br>idx+ | DAV+<br>AI+<br>idx- | DAV+<br>AI+<br>idx+ |
|-----|---------------------|---------------------|---------------------|---------------------|---------------------|---------------------|---------------------|---------------------|
| `POST /` | 403 ✅ | 403 ✅ | 403 ✅ | 403 ✅ | 403 ✅ | 403 ✅ | 403 ✅ | 403 ✅ |
| `POST /test/` | 403 ✅ | 405 ✅ | 403 ✅ | 405 ✅ | 403 ✅ | 405 ✅ | 403 ✅ | 405 ✅ |
| `POST /test` | 301 ✅ | 301 ✅ | 301 ✅ | 301 ✅ | 301 ✅ | 301 ✅ | 301 ✅ | 301 ✅ |
| `POST /fichier.txt` (absent) | 404 ✅ | 404 ✅ | 404 ✅ | 404 ✅ | 404 ✅ | 404 ✅ | 404 ✅ | 404 ✅ |
| `POST /fichier.txt` (présent) | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ |
| `POST /test/fichier.txt` (absent) | 404 ✅ | 404 ✅ | 404 ✅ | 404 ✅ | 404 ✅ | 404 ✅ | 404 ✅ | 404 ✅ |
| `POST /test/fichier.txt` (présent) | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ |

> **POST /** : retourne 403 (pas 405) — nginx ne peut pas déterminer de ressource cible sur un répertoire sans index ni autoindex, quelle que soit la méthode.
> **POST /test/** : seul `idx` importe. 403 si aucun index résolu, 405 si index.html présent. DAV n'a aucun effet sur ce comportement.
> **POST sur fichier** : 404 si absent (DAV ne gère pas POST), 405 si présent. Identique avec ou sans DAV.

---

## PUT — répertoires

| URL | DAV-<br>AI-<br>idx- | DAV-<br>AI-<br>idx+ | DAV-<br>AI+<br>idx- | DAV-<br>AI+<br>idx+ | DAV+<br>AI-<br>idx- | DAV+<br>AI-<br>idx+ | DAV+<br>AI+<br>idx- | DAV+<br>AI+<br>idx+ |
|-----|---------------------|---------------------|---------------------|---------------------|---------------------|---------------------|---------------------|---------------------|
| `PUT /` | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 409 ✅ | 409 ✅ | 409 ✅ | 409 ✅ |
| `PUT /test/` | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 409 ✅ | 409 ✅ | 409 ✅ | 409 ✅ |
| `PUT /test` | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 409 ✅ | 409 ✅ | 409 ✅ | 409 ✅ |

> autoindex et index.html n'affectent pas PUT sur répertoire.
> **`PUT /test` (sans slash) avec DAV+ → 409** : nginx mappe `/test` vers `www/test` qui est un répertoire existant → conflit, même sans trailing slash. Contrairement à l'analyse initiale qui prévoyait 405.

---

## PUT — fichiers

| URL | DAV-<br>AI-<br>idx- | DAV-<br>AI-<br>idx+ | DAV-<br>AI+<br>idx- | DAV-<br>AI+<br>idx+ | DAV+<br>AI-<br>idx- | DAV+<br>AI-<br>idx+ | DAV+<br>AI+<br>idx- | DAV+<br>AI+<br>idx+ |
|-----|---------------------|---------------------|---------------------|---------------------|---------------------|---------------------|---------------------|---------------------|
| `PUT /nouveau.txt` (absent) | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 201 ✅ | 201 ✅ | 201 ✅ | 201 ✅ |
| `PUT /nouveau.txt` (présent) | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 204 ✅ | 204 ✅ | 204 ✅ | 204 ✅ |
| `PUT /test/nouveau.txt` (absent) | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 201 ✅ | 201 ✅ | 201 ✅ | 201 ✅ |
| `PUT /test/nouveau.txt` (présent) | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 204 ✅ | 204 ✅ | 204 ✅ | 204 ✅ |
| `PUT /test/a/b/c.txt` (`create_full_put_path on`) | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 201 ✅ | 201 ✅ | 201 ✅ | 201 ✅ |
| `PUT /test/a/b/c.txt` (`create_full_put_path off`) | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 500 ✅ | 500 ✅ | 500 ✅ | 500 ✅ |

> autoindex et index.html n'affectent pas PUT sur fichier.
> **`create_full_put_path off` → 500** (pas 409 comme attendu) : nginx retourne une erreur interne quand le répertoire parent n'existe pas et que la création automatique est désactivée.

---

## DELETE

| URL | DAV-<br>AI-<br>idx- | DAV-<br>AI-<br>idx+ | DAV-<br>AI+<br>idx- | DAV-<br>AI+<br>idx+ | DAV+<br>AI-<br>idx- | DAV+<br>AI-<br>idx+ | DAV+<br>AI+<br>idx- | DAV+<br>AI+<br>idx+ |
|-----|---------------------|---------------------|---------------------|---------------------|---------------------|---------------------|---------------------|---------------------|
| `DELETE /nouveau.txt` (présent) | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 204 ✅ | 204 ✅ | 204 ✅ | 204 ✅ |
| `DELETE /nouveau.txt` (absent) | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 404 ✅ | 404 ✅ | 404 ✅ | 404 ✅ |
| `DELETE /test/nouveau.txt` (présent) | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 204 ✅ | 204 ✅ | 204 ✅ | 204 ✅ |
| `DELETE /test/nouveau.txt` (absent) | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 404 ✅ | 404 ✅ | 404 ✅ | 404 ✅ |
| `DELETE /` (répertoire racine) | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 204 ✅ | 404 ✅ | 404 ✅ | 404 ✅ |
| `DELETE /test/` (répertoire vide) | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 204 ✅ | 204 ✅ | 204 ✅ | 204 ✅ |
| `DELETE /test/` (répertoire non vide) | 405 ✅ | 405 ✅ | 405 ✅ | 405 ✅ | 204 ✅ | 204 ✅ | 204 ✅ | 204 ✅ |

> autoindex et index.html n'affectent pas DELETE.
> **`DELETE /` avec DAV+ → 204** : nginx supprime le répertoire racine `www/` lui-même. Les 3 tests suivants (idx+, idx-, idx+) retournent 404 car `www/` n'existe plus au moment où ils s'exécutent (artefact de la séquence de test, pas de la config).
> **`DELETE /test/` non vide → 204** : nginx supprime le répertoire récursivement même s'il contient des fichiers.
