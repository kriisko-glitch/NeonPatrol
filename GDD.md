# NeonPatrol — Game Design Document (GAME-017)

> **Genre**: Third-person wave-survival with AI companion
> **Engine**: Unreal Engine 5.6 (C++)
> **Status**: In development
> **Started**: 2026-04-12

---

## The Pitch

You're a patrol guard in a neon-lit sci-fi facility. Your AI companion — a small
combat robot called "Spark" — walks beside you. Malfunctioning security bots
attack in waves. Between fights, you can chat with Spark. Survive together.

---

## Core Loop

```
1. Player + Spark spawn in the patrol facility
2. IDLE: Walk around, explore, chat with Spark
3. WAVE START: Alert sound — hostile bots spawn from doorways
4. COMBAT: Player shoots, Spark fights alongside
5. WAVE CLEAR: All enemies dead → 30-second cooldown
6. Score increments. Next wave is harder (more/tougher bots)
7. Repeat until player dies
```

---

## Player Character

- Third-person camera (Spring Arm + Camera)
- WASD movement, mouse look
- Left-click: Shoot projectile (simple energy ball)
- Health: 100 HP, regenerates slowly out of combat
- Death: Ragdoll → "Game Over" screen → restart

## Spark (AI Companion)

- Small hovering robot (placeholder: sphere mesh with emissive material)
- Follows player at ~300 units distance
- In combat: shoots at nearest enemy
- Between waves: wanders nearby, reacts to player
- Chat: Player presses T to open chat, types message, Spark responds via LLM
- LLM backend: Qwen 0.8B on 127.0.0.1:11001

### Spark Personality (System Prompt)
```
You are Spark, a small combat robot companion. You're loyal, slightly sarcastic,
and brave. You love fighting alongside your patrol partner. Keep responses under
20 words. You're in a sci-fi facility fighting malfunctioning robots.
```

## Robot Enemies

- **Drone** (basic): Low HP, slow, melee attack. Placeholder: cube mesh.
- **Turret** (ranged): Medium HP, stationary, shoots projectiles.
- **Heavy** (tank): High HP, slow, charges at player.

### Wave System
- Wave 1: 3 Drones
- Wave 2: 4 Drones + 1 Turret
- Wave 3: 5 Drones + 2 Turrets
- Wave N: scales linearly. Every 5th wave adds a Heavy.
- 30-second cooldown between waves
- Enemies spawn from designated spawn points around the arena

## Environment

- Modular sci-fi facility (Sci_fi_hallway Fab pack)
- One main arena room with corridors leading to spawn points
- Neon lighting (blue ambient, red accents at spawn points)
- For Phase 1: use default Third Person template map
- Later: build proper arena from Fab assets

## HUD

- Health bar (top-left)
- Wave counter (top-center)
- Score (top-right)
- Chat panel (bottom, toggled with T key)
- Crosshair (center)

---

## Technical Architecture

### C++ Modules

**NeonPatrol module** (gameplay):
| File | Purpose |
|------|---------|
| NeonPatrolCharacter.h/cpp | Player character with health, shooting |
| SparkCharacter.h/cpp | Companion AI pawn |
| SparkBrainComponent.h/cpp | LLM integration (HTTP → Qwen) |
| RobotEnemy.h/cpp | Base enemy class |
| WaveSpawner.h/cpp | Spawns enemies in waves |
| CombatComponent.h/cpp | Health, damage, death (shared) |
| Projectile.h/cpp | Simple energy projectile |
| ChatWidget.h/cpp | UMG chat UI |
| NeonPatrolGameMode.h/cpp | Orchestrates everything |
| NeonPatrolHUD.h/cpp | Wave/score/health display |

**TP_ThirdPerson module** (from template — minimal edits):
- Provides base character, input, camera

### Dependencies
- HTTP, Json, JsonUtilities (LLM communication)
- AIModule, NavigationSystem (enemy + companion AI)
- EnhancedInput (player input)
- UMG, Slate, SlateCore (UI)

---

## Development Phases

### Phase 1 — Walk Around (MVP)
- Player character with movement in template map
- Basic camera and input
- **Playable**: You can walk around a space

### Phase 2 — Got a Buddy
- Spark companion spawns and follows player
- Basic idle/follow behavior
- **Playable**: You walk around with a companion

### Phase 3 — Fight!
- Robot enemies spawn in waves
- Projectile shooting
- Health system + death
- Wave counter + score
- **Playable**: A real game with combat

### Phase 4 — Talk to Spark
- Chat UI (press T)
- LLM integration with Qwen 0.8B
- Spark responds to player messages
- **Playable**: Full experience with AI chat

---

## Success Criteria

- [ ] Player can walk around a 3D environment
- [ ] Spark follows the player naturally
- [ ] Enemies spawn in waves and can be killed
- [ ] Player can die and restart
- [ ] Player can chat with Spark and get LLM responses
- [ ] Compiles clean on UE5 5.6
- [ ] Feels like a game someone would play for 5 minutes
