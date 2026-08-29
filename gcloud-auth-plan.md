# Plan: GCloud Authentication Inside Docker Container

## Overview

The host machine cannot run scripts, so `gcloud auth application-default login` cannot be
executed outside of Docker. The solution is to:

1. Add a named Docker volume to persist gcloud credentials across container restarts.
2. Add a dedicated one-time login service (`translate-login`) that opens an interactive
   shell for the user to authenticate via browser code — no host scripting required.
3. Update the existing `translate` service to use the named volume and remove the broken
   host path mount (`${APPDATA}/gcloud`).

After the one-time login, normal `docker compose up translate` runs will find credentials
in the named volume automatically.

---

## Sub-Tasks

### Sub-Task 1 — Replace host credentials mount with a named volume

**Intent**
The current volume `${APPDATA}/gcloud:/root/.config/gcloud` relies on credentials existing
on the host, which they do not. A named Docker volume persists credentials inside Docker
itself, decoupling the setup from the host filesystem entirely.

**Expected Outcomes**
- `${APPDATA}/gcloud:/root/.config/gcloud` is removed from the `translate` service volumes.
- A named volume `gcloud-config` is mounted at `/root/.config/gcloud` in the `translate` service.
- A top-level `volumes:` block declares `gcloud-config:` so Docker manages it.

**Todo List**
1. In the `translate` service `volumes:` block, replace `- ${APPDATA}/gcloud:/root/.config/gcloud` with `- gcloud-config:/root/.config/gcloud`.
2. Add a top-level `volumes:` block at the end of `docker-compose.yaml` declaring `gcloud-config:`.

**Relevant Context**
- [`docker-compose.yaml`](docker-compose.yaml) lines 9–10 (current volumes block)
- [`docker-compose.yaml`](docker-compose.yaml) line 33 (end of file, where top-level volumes block is added)

**Status**
[x] done

---

### Sub-Task 2 — Add a one-time interactive login service

**Intent**
The user needs a way to run `gcloud auth application-default login --no-browser` inside a
container interactively. A separate `translate-login` service keeps this concern isolated
from the normal `translate` service, and only needs to be run once.

**Expected Outcomes**
- A new `translate-login` service is added to `docker-compose.yaml`.
- The service uses the same `gcr.io/google.com/cloudsdktool/google-cloud-cli` image.
- It mounts the `gcloud-config` named volume at `/root/.config/gcloud`.
- It sets `stdin_open: true` and `tty: true` for interactive use.
- Its `command` runs `gcloud auth application-default login --no-browser`.
- Running `docker compose run translate-login` prompts the user with a URL and a code input — no host scripting required.

**Todo List**
1. Add a `translate-login` service block in `docker-compose.yaml` after the `translate` service.
2. Set `image: gcr.io/google.com/cloudsdktool/google-cloud-cli`.
3. Mount `gcloud-config:/root/.config/gcloud`.
4. Set `stdin_open: true` and `tty: true`.
5. Set `command: gcloud auth application-default login --no-browser`.
6. Connect it to `app-net`.

**Relevant Context**
- [`docker-compose.yaml`](docker-compose.yaml) — new service block added after line 17
- Credentials written by this service go into the `gcloud-config` named volume, which the `translate` service also mounts

**Status**
[x] done

---

### Sub-Task 3 — Update the translate service command to remove the inline token assignment

**Intent**
The current `command` uses `GCP_ACCESS_TOKEN=$$(gcloud ...) &&` but the `&&` operator
means the variable is never passed to `go run` — it just evaluates and discards the
assignment. With credentials now available via the named volume, the correct form is an
inline prefix: `GCP_ACCESS_TOKEN=$(gcloud ...) go run ./translate/` (no `&&`).

**Expected Outcomes**
- Line 15 of `docker-compose.yaml` no longer uses `&&` after the token assignment.
- The token is assigned as an inline environment variable prefix directly to `go run`.
- `go run ./translate/` can read `GCP_ACCESS_TOKEN` via `os.Getenv`.

**Todo List**
1. In the `translate` service `command`, change:
   `GCP_ACCESS_TOKEN=$$(gcloud auth application-default print-access-token) &&`
   to:
   `GCP_ACCESS_TOKEN=$$(gcloud auth application-default print-access-token)`
   (remove the trailing `&&` so it becomes an inline prefix to the next line).

**Relevant Context**
- [`docker-compose.yaml`](docker-compose.yaml) lines 12–17 (command block)
- [`translate/translate.go`](translate/translate.go:31) — reads `GCP_ACCESS_TOKEN` via `os.Getenv`

**Status**
[x] done
