# Contributing to WasEngine

This project uses a simple feature-branch + pull request workflow so every
change is built by CI (GitHub Actions) before it lands on `main`.

## Branch naming

Create a new branch off `main` for every change, using one of these prefixes:

| Prefix      | Use for                                      | Example                          |
|-------------|-----------------------------------------------|-----------------------------------|
| `feature/`  | New functionality                             | `feature/pyramid-renderer`        |
| `fix/`      | Bug fixes                                     | `fix/window-resize-crash`         |
| `chore/`    | Tooling, build, CI, docs, cleanup             | `chore/add-github-actions`        |
| `refactor/` | Code restructuring with no behavior change    | `refactor/renderer-cleanup`       |

## Workflow

1. Sync `main` and create your branch:
   ```powershell
   git checkout main
   git pull
   git checkout -b feature/my-change
   ```
2. Make your changes and commit them with clear, descriptive messages.
3. Build locally before pushing (fail fast, don't rely on CI alone):
   ```powershell
   make build
   ```
   or, without `make`:
   ```powershell
   msbuild WasEngine.sln /p:Configuration=Debug /p:Platform=x64
   ```
4. Push the branch and open a pull request into `main`:
   ```powershell
   git push -u origin feature/my-change
   ```
5. Wait for the **Build** GitHub Actions workflow to pass on the PR (builds both
   `Debug` and `Release`, `x64`). Fix any failures before merging.
6. Merge the PR once CI is green. Prefer **Squash and merge** to keep `main`'s
   history clean, unless the branch has meaningful individual commits worth
   preserving.
7. Delete the feature branch after merging (GitHub can do this automatically —
   see repo Settings → General → "Automatically delete head branches").

## Branch protection

`main` is (or should be) configured to require the CI status checks to pass
before a PR can be merged. See the repo's branch protection rules under
**Settings → Rules → Rulesets** for `main`.

## Commit message style

Keep the first line short and imperative (e.g. `Add pyramid renderer`, `Fix
resize crash`), with optional further detail in the body if needed.
