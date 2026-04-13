# NeonPatrol — GAME-017
> Third-person wave-survival with AI companion
> Engine: Unreal Engine 5.6 (C++)
> Started: 2026-04-12

---

## What This Is

A third-person action game forked from Epic's UE5 ThirdPerson-Combat template.
Player patrols a sci-fi facility with an AI companion robot ("Spark").
Malfunctioning robots attack in waves. Survive together, chat between fights.

## Architecture

Two C++ modules:

**TP_ThirdPerson** (from Epic template):
- Base character, camera, input
- Combat variant: melee combat, damage system, enemy AI, spawner
- StateTree-based enemy behavior
- All the ICombatDamageable / ICombatAttacker interfaces

**NeonPatrol** (our gameplay):
- SparkCharacter — AI companion that follows + fights
- SparkBrainComponent — LLM chat via HTTP → Qwen 0.8B (port 11001)
- WaveSpawner — wave-based enemy spawning (extends template spawner concept)
- NeonPatrolCharacter — player with projectile shooting + health
- NeonPatrolGameMode — orchestrates waves, companion, scoring
- ChatWidget — in-game chat UI
- NeonPatrolHUD — health, wave, score display
- CombatComponent — shared health/damage component
- Projectile — simple energy projectile
- RobotEnemy — enemy character (type: Drone/Turret/Heavy)

## Key Paths

| What | Path |
|------|------|
| GDD | `GDD.md` |
| Project file | `NeonPatrol.uproject` |
| Our gameplay code | `Source/NeonPatrol/` |
| Template code | `Source/TP_ThirdPerson/` |
| Combat variant | `Source/TP_ThirdPerson/Variant_Combat/` |
| Build config | `Config/DefaultEngine.ini` |
| Content | `Content/` |

## Build

```bash
# Compile (Development Editor)
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" \
  TP_ThirdPersonEditor Win64 Development \
  "D:\Games\NeonPatrol\NeonPatrol.uproject" -NoUBA

# The Target name is TP_ThirdPersonEditor (from the template Target.cs)
```

## LLM Integration (Spark Chat)

- Endpoint: `http://127.0.0.1:11001/v1/chat/completions`
- Model: Qwen 0.8B on local 4060 GPU
- Required: `chat_template_kwargs: {"enable_thinking": false}`
- Required: `User-Agent: Kriisko-Studio/1.0`
- Round-trip: ~15ms warm

## Development Phases

1. **Walk Around** (template gives us this) — DONE via fork
2. **Combat** (template gives us this) — DONE via fork
3. **Companion AI** — SparkCharacter follows + fights alongside
4. **Wave System** — WaveSpawner with scaling difficulty
5. **Chat with Spark** — LLM-powered conversation

## Git

- Repo: https://github.com/kriisko-glitch/NeonPatrol
- Branch: main
- Commit footer: `Authored-By: Kriisko-Studios <kriisko-glitch@users.noreply.github.com>`
