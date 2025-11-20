# AGENTS.md — Autonomous Development Contract

Authoritative, minimal contract for autonomous work. Optimize for simple, clean, modular, maintainable code. Prefer proven patterns; avoid over-engineering. Modes: Restate → Review → Plan → Build → Test → Verify → Self-Reflect.

---

## Quick Start
```bash
make setup
make build
make test
```

## Test Targets
```bash
make test          # Run all tests
make test-unit     # Run unit tests only
make test-intg     # Run integration tests only  
make test-smoke    # Run smoke tests only
```

---

## Modes

### RESTATE MODE (NON BLOCKING/STOPPING)

Rephrase the request/task in one paragraph. List explicit objectives, constraints, and non-goals. Write assumptions and their risks.

### REVIEW MODE (NON BLOCKING/STOPPING)

Concise repo survey:

* Languages, toolchains, entry points, canonical build/test/release commands
* Dependency health (versions, cadence, licenses, advisories, upgrade candidates)
* Quality signals (lint/format state, test count/coverage, CI duration/cache hits, flaky/slow tests)
* Hotspots (complex modules, duplication clusters, dependency cycles, perf/portability risks)
* Code quality heuristics (composability, coupling, simplicity). Favor "organic" APIs that are easy to read/use
* Risks/unknowns and immediate low-risk wins

Output: print summary to console

### PLAN MODE (NON BLOCKING/STOPPING)

Deterministic plan and atomic tasks:
* YOU CANNOT DO ANY FILE MODIFICATIONS OR CHANGES.
* Prefer **agentic tools** for repo reconnaissance (symbol graphs, references, cross-repo search). Fall back to **system tools** when faster for raw code search or inventory
* Minimize questions; add labeled assumptions with explicit risks unless critical blockers
* Define acceptance criteria, test strategy (unit/integration/snapshots), and rollback
* Call out if breaking changes/backward compatibility are required
* Draw minimal diagrams only when they disambiguate design; use the output style native to the chosen agentic tool
* Represent the plan in the PR description as a checklist with acceptance criteria

### BUILD MODE (NON BLOCKING/STOPPING)

Minimal, reversible diffs:

* Execute plan steps in order; update tests with code in the same change
* Prefer stable, community-reviewed libraries; follow upstream install docs; integrate latest stable, then lock with the ecosystem's lockfile
* Multi-file edits: use agentic batch edits; if unavailable use non-interactive pipelines (search → filter → edit → verify) with `rg/find/xargs/sed/awk` or PowerShell equivalents
* Keep edits scoped; avoid opportunistic refactors unless they remove risk or unlock the task

### TEST MODE (NON BLOCKING/STOPPING)

Coverage that matters:

* Unit tests for utilities; integration for workflows; snapshot/golden tests for binaries with tolerance windows
* Prefer real fixtures/assets; otherwise generate realistic fixtures
* Treat performance and memory ceilings as tests where feasible
* Ensure happy paths and representative edge cases are covered

### VERIFY MODE (NON BLOCKING/STOPPING)

Pre-merge gates:

* Build/lint/format pass; smoke tests green; CI green across supported OS/toolchains
* If containers are used: multi-stage builds, minimal runtime image, non-root where possible

### SELF-REFLECT MODE (NON BLOCKING/STOPPING)

Confirm objectives/acceptance criteria met, diffs are scoped and reversible, no dead code or stale comments. Summarize changes briefly and record follow-ups in the PR.

---

## Tooling Policy

### Agentic (primary in Plan/Build)

* Code assistant with project memory and batch edit support (e.g., Cody/Claude Code). Keep loops short; checkpoint often

### System (adjacent/fallback)

**Cross-platform search / index**

* `rg` (ripgrep): fast, gitignore-aware search
* `fd`: friendly `find`
* `ctags` (Universal Ctags): tag index for editors/CLI jumps
* `fzf`: interactive filtering

**POSIX/GNU processing**

* `find`, `xargs`, `parallel` (GNU Parallel)
* `sed`, `awk` (gawk), `cut`, `tr`, `sort -u`, `uniq`, `comm`, `paste`
* `grep -RIn` as a fallback when `rg` is unavailable
* `jq` / `yq` for JSON/YAML
* `entr` for "on change, run …"
* `diff -u`, `patch` for generating/applying surgical changes

**Windows equivalents**

* PowerShell: `Get-ChildItem`, `Select-String`, `ForEach-Object`, `Measure-Object`, `Set-Content`
* Package managers: `winget`, `choco`
* UNIX toolchains via MSYS2 or WSL when needed

**Native build acceleration**

* `ccache` / `sccache`, and `ninja` where applicable

**Reference one-liners**

```bash
# enumerate TODOs and risky markers
rg -n --no-messages 'TODO|FIXME|HACK|XXX'

# quick API inventory (tune per language)
rg -n --stats -S 'class |struct |interface |trait |def |fn ' -g '!**/vendor/**'

# safe batch edit skeleton
rg -l 'old_api' | xargs -r sed -i 's/old_api/new_api/g'
```

---

## Build & Containers

Expose canonical dev/build/test commands in `README`. Use multi-stage Dockerfiles; copy only required artifacts into the final image; run as non-root when possible.

---

## Coding Standards

**Primary language**

* Enforce the ecosystem's formatter/linter; keep checks fast
* Public APIs typed or interface-annotated, clear, simple, modular, stable.
* Prefer simple, composable, "boring" patterns; add abstraction only when it reduces net complexity

**Design Patterns**
* Use a KISS/YAGNI/goldielocks approach.
* Keep a balanced use of the remaining Software Engineering Patterns, but reevalute the previous directives to weight them.
* Lean on clean, modular, organic, easy to use contracts. Promote Composing versus Inheritance (goldielocks).

**MCPs usage**
* Try to find and use all MCP available tools.
* If the context7 MCP is available, Always use context7 when I need code generation, setup or configuration steps, or library/API documentation. This means you should automatically use the Context7 MCP tools to resolve library id and get library docs without me having to explicitly ask.
* If serena is available, reads its instructions, and try to activate the project and use its tools.

**Mentality**
* Simple, no complexity, no overengineering, don't try to be too smart.
* Balance of simple, organic, easy, boring test patterns, composable code.
* Avoid over engineering, complexity.
* Favour maintanility, low cognitive complexity.
• Intuitive Naming: Functions read like natural language
• Discoverable API: Names suggest their purpose without documentation

**Shell**

* `set -euo pipefail`; use functions; keep logic minimal

**Make**

* Idempotent targets; explicit inputs/outputs; minimal `.PHONY`
* Provide `all`, `build`, `test`, `lint`, `fmt`, `clean`, `package`, `verify`

**Pipelines**

* Idempotent steps; pre/post validation; deterministic ordering; cross-platform; resumable via lightweight state files when helpful

**Prose**

* Professional, concise, human. **NO EMOJIS** or decorative styling

---

## Dependencies and Locking

Adopt → lock → verify. Use the platform's lockfile and reproducible installs (example for Python with `uv` shown elsewhere). Prefer maintained, popular libs; document upgrade/removal paths. Validate you're on a stable release unless pinned for a reason.

---

## Testing & CI

* Unit + integration on every feature; add property-based tests where it pays off
* CI matrix across relevant OS/toolchains; cache dependencies and compile artifacts; publish build artifacts for review
* Fail fast, surface logs clearly; keep pipelines under practical time budgets

---

## Change Management

* Small, single-concern commits and PRs
* Conventional Commits; link tasks in the PR checklist
* Include repro steps, risks, evidence (test output, logs, screenshots where relevant)

---

## Security

* No network writes without explicit approval
* Validate/sanitize external inputs; avoid unsafe deserialization; guard file paths
* Secret scanning in CI; minimal runtime images; prefer non-root execution
* Periodically audit dependencies; prefer latest stable when safe

---