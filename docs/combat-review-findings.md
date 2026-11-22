# Combat systems review findings

Findings from a full review of `main`, tracked as GitHub issue #47 (parent) with sub-issues #48 to #57. Work each item on its own topic branch off `main`, test first, one PR per issue, closing the issue with a closing keyword.

## Context

- `APawn::TakeDamage` refuses all damage in a world with no game mode, so a test that goes through it needs `FCombatWorldFixture::MakeCombatWorld(true)`.
- Tests run headless with `UnrealEditor-Cmd.exe`; results are read from `Saved/Logs/FirstCplusProject.log`.
- Some findings were reported by review agents and not re-verified by hand. Start each item by reading the cited lines.

## Test integrity

- [ ] **`DeadEnemyTakesNoDamage` must fail without the `Alive()` guard** (#48, `DeathGuardTests.cpp:53-66`)
  - Why: the test uses a world with no game mode, so `TakeDamage` returns 0 on its own and removing the guard at `Enemy.cpp:66-69` still passes.
  - Test first: use `MakeCombatWorld(true)` plus a damage listener, kill the enemy, apply damage, assert nothing fired and health stays 0. Remove the guard to show it failing, then restore it.

- [ ] **Close coverage gaps and add a fixture scope guard** (#54, `Source/FirstCplusProjectTests/Private`)
  - Why: enemy states, sprint and stamina, `Equip` and overlaps have no tests, and a failing test can leak the world because teardown has no scope guard.
  - Test first: one test per area, plus a deliberately failing assertion that must still destroy the world. Add a scope-guard struct to the fixture.

## Combat correctness

- [ ] **Set attack flags only after the montage plays** (#49, `MainCharacter.cpp:524-559`, `Enemy.cpp:350-366`)
  - Why: `bAttacking` and `bCanAttack` are set before the montage guard, so a missing montage blocks all later attacks.
  - Test first: clear `CombatMontage`, call `Attack()`, assert the flag is unchanged, for both classes.

- [ ] **Stop enemies attacking a dead player** (#50, `Enemy.cpp:211-216, 256-270, 413-438`, `MainCharacter.cpp:110-144`)
  - Why: enemies keep scheduling and landing attacks after the player dies.
  - Test first: with `bIsDead` set on the player inside the combat sphere, assert no montage plays and no timer is set. Clear the combat target on death.

- [ ] **Clear `ActiveOverlappingItem` only for the item that ended overlap** (#51, `Weapon.cpp:66-75`)
  - Why: unconditional clearing drops a different item the player still overlaps.
  - Test first: overlap items A and B, end overlap of A, assert B is still active.

- [ ] **Share one notify-name normalization** (#52, `Enemy.cpp:171-182`, `MainCharacter.cpp:200-217`)
  - Why: the player strips spaces from notify names and the enemy compares them raw.
  - Test first: feed a spaced name to both and assert the same handler runs.

## Robustness and config

- [ ] **Null-harden `SpawnVolume`, `Critter` and `Collider`** (#53, `SpawnVolume.cpp:39-46`)
  - Why: a raw `UClass*` with `CastChecked`, and missing null checks on dereferenced pointers.
  - Test first: leave the spawn class unset and assert no crash and no actor. Use `TSubclassOf` and null checks.

- [ ] **Remove the tracked editor token and restrict remote execution** (#55, `Config/DefaultEngine.ini`, `FirstCplusProject.uproject`)
  - Why: an editor pairing token is tracked in git, and Python remote execution is bound to `0.0.0.0` with developer mode; `RemoteControl` and `PythonScriptPlugin` have no `TargetAllowList`.
  - Change: move the token to an untracked per-user config and rotate it, bind remote execution to loopback or disable it, and limit both plugins to the editor. Never print the token value. Non-behavioral, so verify by grep and by opening the editor.

## Cleanup and docs

- [ ] **Remove dead code and fix small items** (#56)
  - Why: a commented-out debug block (`Enemy.cpp:324-330`), empty overrides and duplicated collision toggles; `Enemy::MoveToTarget` and `AttackEnd` lack `Alive()` guards; `MainCharacterAnimInstance.cpp:36` has no null check; both AnimInstances skip `Super::NativeInitializeAnimation`.
  - Test first: a dead enemy's `MoveToTarget` does nothing, and a null-owner `UpdateAnimationProperties` does not crash.

- [ ] **Refresh `.github/copilot-instructions.md`** (#57)
  - Why: it still says UE 5.7, `BuildSettingsVersion.V5`, three enemy states and no automation tests.
  - Change: through `/update-copilot-instructions`, in its own PR.

## Open questions

- Does `Collider` `CameraInput` really never reset? Confirm in PIE before fixing (#53).
- `FloorSwitch` bool versus counter, pickup coins lost at the cap, and `Explosive` bypassing `TakeDamage`: design choice or defect?
- Hands-on PIE checks of the combat loop still need someone in the editor.
