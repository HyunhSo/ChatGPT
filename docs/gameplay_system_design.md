# Gameplay System Design (UE5, Lyra-like Modular Style)

## 1) High-level system overview

Use a **feature-sliced modular gameplay architecture** inspired by Lyra:

- **Experience layer** boots the match and assembles gameplay features from data.
- **Core runtime layer** (GameMode, GameState, PlayerState, Pawn/Controller) owns lifecycle and network authority.
- **Feature components** (combat, movement rules, abilities, inventory, interaction) attach to runtime actors and activate through init-state orchestration.
- **Data-driven definitions** (pawn data, experience definitions, feature configs) decide which components/classes are active without hardcoding dependencies.

Design goal: each gameplay feature can be added incrementally as a new module/component with minimal edits to existing systems.

---

## 2) Major components and responsibilities

### A. Experience & feature activation

- **Experience Definition (data asset)**
  - Declares game phase rules, default pawn data, feature sets, and optional game mode policies.
  - Should contain references only to abstract interfaces/data assets where possible.

- **Experience Manager / Loader (GameState-side authoritative sync)**
  - Loads the selected experience and broadcasts readiness milestones (Loaded, Activated).
  - Ensures all clients converge on the same feature set before gameplay starts.

### B. Actor initialization framework

- **Pawn Extension Component (on Pawn)**
  - Central init-state machine for pawn-level feature components.
  - Tracks dependencies (e.g., "Ability System Ready" before "Hero Ready").

- **Hero/Player bootstrap component (on Pawn or Controller)**
  - Applies pawn data, input config, camera mode, default ability/equipment grants.
  - Bridges player identity (PlayerState) to controllable character state.

### C. Combat/gameplay feature components

- **Combat Component**
  - Owns high-level combat state and delegates details to sub-systems (weapon handling, targeting, cooldown windows).
  - Exposes a stable interface/events to UI and animation systems.

- **Weapon/Equipment Component or Weapon actor abstraction**
  - Handles equip/unequip lifecycle, fire requests, ammo/reload state.
  - Server-authoritative validation for fire/equip actions.

- **Interaction/Inventory/etc. components (future or optional)**
  - Independent feature units following the same init and messaging contracts.

### D. Game framework authorities

- **GameMode (server only)**
  - Match rules, spawning policy, win/loss evaluation, experience selection.
  - Should not hold client UI or per-player cosmetic state.

- **GameState (replicated)**
  - Match-shared replicated state: phase, timers, score model, active experience info.

- **PlayerState (replicated)**
  - Persistent player identity/session data across pawn respawns: team, progression stats, selected loadout profile.

- **PlayerController (owner + server bridge)**
  - Input intent submission and player-specific RPC boundaries.
  - Owns local-only presentation orchestration hooks (HUD creation triggers), not raw gameplay truth.

---

## 3) Ownership rules (who owns what, who should not know what)

### Authority ownership

- **GameMode owns rules**; no client-side system should decide authoritative outcomes.
- **GameState owns shared replicated match snapshot**; avoid duplicating global truth in multiple actors.
- **PlayerState owns persistent player gameplay identity**; pawn owns transient body state.
- **Feature components own their own state machines**; other systems interact through interfaces/events.

### Knowledge boundaries

- Components should depend on:
  - Interfaces
  - Gameplay tags/events
  - Data assets
  - Subsystem/service APIs

Avoid direct deep references like:

- UI widget calling weapon internals directly.
- Combat component knowing concrete GameMode subclass details.
- Inventory component mutating ability system internals without an explicit service/interface.

### Replication boundaries

- Server is source of truth for gameplay-critical state.
- Clients may predict input outcomes, but reconciliation flows back from authoritative components.
- Local-only systems (camera shake, non-gameplay VFX, menu state) must not feed authoritative logic.

---

## 4) Data flow between components

Typical lifecycle flow:

1. **Match start (server):** GameMode chooses Experience Definition.
2. **Experience load:** GameState/Experience Manager replicates chosen experience and activation state.
3. **Player spawn:** GameMode spawns pawn; PlayerState is attached/associated.
4. **Init orchestration:** Pawn Extension Component advances init tags as dependencies become ready.
5. **Feature activation:** Hero + Combat + Weapon components read pawn data and register themselves.
6. **Input flow:** Local input -> PlayerController -> server RPC/ability input channel -> feature component.
7. **Authoritative resolution:** Server validates and mutates state (damage, ammo, cooldown).
8. **Replication/event broadcast:** Updated state replicates; UI/animation subscribe to events and refresh.

Recommended communication mechanisms:

- **Gameplay tags + message bus style events** for loose coupling.
- **Interfaces for request/response contracts** between features.
- **Data assets for static config** (no hardcoded tuning constants in component logic).

---

## 5) Future extension points

Design in explicit extension seams:

- **New feature packs via experience action sets**
  - Add traversal, gadgets, objective modes without editing base pawn class.

- **Policy objects / strategy assets**
  - Spawn policy, scoring policy, damage policy can be swapped per mode.

- **State-tree or ability-task integration**
  - Complex AI/player action flow can move from ad-hoc code to reusable behavior assets.

- **Cross-cutting services**
  - Telemetry, replay markers, anti-cheat hooks, matchmaking metadata mappers.

- **Tooling hooks**
  - Debug UI for init-state graph, replicated state inspectors, feature activation timeline.

---

## 6) Common pitfalls to avoid

- **Monolithic "god" character/component**
  - Symptom: one class owns movement, combat, inventory, interaction, UI bridges.
  - Fix: split by feature + explicit contracts.

- **Hard references between sibling features**
  - Symptom: Combat directly instantiates Inventory internals.
  - Fix: interface/event-driven boundary with dependency declaration.

- **Putting persistent player data on Pawn**
  - Breaks on respawn/travel; use PlayerState.

- **Skipping init-state gating**
  - Race conditions at spawn (input arrives before abilities/equipment ready).
  - Always gate behavior on explicit readiness tags.

- **Client-authoritative gameplay shortcuts**
  - Creates desync and exploit vectors; keep client prediction cosmetic/responsive but server authoritative.

- **Unversioned data assets**
  - Add schema/version fields and migration notes for long-lived projects.

- **No observability**
  - Without logs/trace categories and debug widgets, modular systems become hard to reason about.
