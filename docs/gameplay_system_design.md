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

## 2.5) Component boundary definitions (Hero / Movement / Stamina / Input)

The following boundaries assume a modular, Lyra-style pawn stack where each component has a narrow, testable contract.

### A) MLHeroComponent

**Core responsibilities**

- Orchestrates hero-level startup/shutdown for a pawn ("hero ready" gate).
- Applies hero/pawn data (movement profile, ability sets, input config references) through services, not hardcoded values.
- Wires high-level dependencies between sibling feature components (movement, stamina, combat), primarily through init-state tags/events.
- Owns hero-level replicated status that must survive component ordering differences but not pawn destruction.

**Allowed to know**

- Pawn Extension init-state API and readiness tags.
- Abstract interfaces of sibling systems (e.g., `IMLMovementInterface`, `IMLStaminaInterface`, input config provider).
- PlayerState identity handle needed to bind persistent player data to this pawn instance.
- Data assets that define hero defaults.

**Must NOT know**

- Low-level movement math (acceleration curves, friction formulas).
- Raw stamina drain/regeneration formulas.
- Concrete input device mappings or UI widget internals.
- GameMode rule details beyond exposed policy interfaces.

**Public-facing vs internal logic**

- Public-facing:
  - `InitializeHero(...)`, `UninitializeHero(...)`, readiness/query accessors, and high-level hero state events (`OnHeroReady`, `OnHeroShutdown`).
- Internal:
  - Dependency tracking, one-time grant/apply guards, and ordering/race protection between component init phases.

**Typical lifecycle events**

- `OnRegister` / `BeginPlay`: register required dependencies and init tags.
- `OnPawnReadyToInitialize` (or equivalent extension hook): apply hero data.
- `OnPossessedBy` / `OnRep_PlayerState`: bind player identity.
- `EndPlay` / `OnUnPossessed`: revoke transient grants and clear listeners.

### B) MLMovementComponent

**Core responsibilities**

- Converts movement intent into movement state transitions (walk/sprint/crouch/dash eligibility and active mode).
- Owns movement capability flags and local movement-related cooldown windows.
- Publishes movement state changes for animation/UI consumers.
- Requests stamina costs through stamina interface when movement actions require resource checks.

**Allowed to know**

- Character movement primitive/component API.
- Input intent abstraction (`Move`, `SprintPressed`, `CrouchToggle`) but not concrete device bindings.
- Stamina service/interface for "can pay" and "consume" style calls.
- Movement configuration assets/tuning tables.

**Must NOT know**

- Player progression persistence model (belongs to PlayerState/profile systems).
- UI implementation specifics (widgets, HUD layouts).
- Match scoring/win conditions.
- Directly mutating stamina internals without going through the stamina contract.

**Public-facing vs internal logic**

- Public-facing:
  - Requests like `SetMoveInput`, `TryStartSprint`, `TryStopSprint`, `GetMovementState`, and movement state delegates/events.
- Internal:
  - Speed blending, acceleration gating, anti-spam timers, prediction helpers, and authoritative reconciliation details.

**Typical lifecycle events**

- `InitializeComponent` / `BeginPlay`: cache movement primitive and config.
- Per-frame update (`TickComponent`) for mode evaluation and transitions.
- Network callbacks (`OnRep_*`) for replicated movement mode/capability flags.
- `EndPlay`: clear timers/delegates.

### C) MLStaminaComponent

**Core responsibilities**

- Owns stamina resource state (current, max, regen delay, regen rate).
- Validates and processes stamina spend requests from other components.
- Drives regeneration rules and interruption windows.
- Replicates gameplay-critical stamina state and emits change events.

**Allowed to know**

- Time/update source needed for regen and cooldown tracking.
- Abstract requesters through tags/reason codes (e.g., sprint, dodge, heavy attack).
- Authority/prediction model required for spend validation.
- Tuning data assets for stamina parameters.

**Must NOT know**

- Exact movement implementation details (velocity, friction).
- Input binding or input action asset details.
- UI rendering logic beyond broadcasting data/events.
- Weapon/combat internals except as generic resource consumers.

**Public-facing vs internal logic**

- Public-facing:
  - `CanSpend`, `Spend`, `Restore`, `GetStaminaSnapshot`, `OnStaminaChanged`, `OnStaminaDepleted`.
- Internal:
  - Clamp/math logic, regen timers, spend queueing, server/client reconciliation rules.

**Typical lifecycle events**

- `BeginPlay`: initialize current/max values from config.
- `TickComponent` or timer-driven loop for regeneration.
- Replication notifications for stamina values and regen lockouts.
- `EndPlay`: stop timers and clear observers.

### D) MLInputComponent

**Core responsibilities**

- Binds input actions to gameplay intent messages/commands.
- Translates hardware/input-framework events into semantic gameplay actions.
- Routes intent to owning controller/pawn feature interfaces in a predictable, remappable way.
- Handles local input enable/disable gates based on hero readiness and gameplay state.

**Allowed to know**

- Enhanced Input (or equivalent) mapping contexts and action assets.
- Owning controller/pawn interfaces that accept intent commands.
- Local player-specific settings for sensitivity/keybind layers.

**Must NOT know**

- Stamina formulas, movement physics math, or combat damage logic.
- Authoritative game rule decisions.
- Persistence schema for progression/loadout (except requesting application through a service).

**Public-facing vs internal logic**

- Public-facing:
  - `BindInput`, `UnbindInput`, mapping-context management, and intent dispatch callbacks/events.
- Internal:
  - Trigger buffering, chord/combo interpretation, dead-zone shaping, device-switch heuristics.

**Typical lifecycle events**

- `SetupPlayerInputComponent`: bind actions and mapping contexts.
- `OnPossess` / `OnRep_Controller`: rebind on control changes.
- `OnHeroReady` / `OnHeroShutdown`: gate or ungate gameplay input.
- `EndPlay`: unbind and cleanup local delegates.

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
