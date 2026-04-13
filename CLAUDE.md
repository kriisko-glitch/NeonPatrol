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

**NeonPatrol** (our gameplay — 28 C++ files):
- SparkCharacter — AI companion, follows/fights/obeys commands
- SparkBrainComponent — LLM chat via Groq API (llama-3.3-70b)
  - JSON command parsing: {"say":"text", "cmd":"COMMAND PARAM"}
  - 12 commands: FOLLOW, STAY, ATTACK, HOLD_FIRE, MOVE_*, AGGRESSIVE, DEFENSIVE, SCOUT
- SparkVoiceComponent — STT (Groq Whisper) + TTS (Windows SAPI) via audio bridge
- SparkCommentaryComponent — 40+ contextual combat quips, idle chatter
- ChatOverlayWidget — bottom-left chat UI, Enter to toggle, programmatic UMG
- NeonPatrolChatSubsystem — auto-creates UI, Enter/V key input polling
- WaveSpawner — wave-based enemy spawning with difficulty scaling
- NeonPatrolCharacter — player with projectile shooting + health
- NeonPatrolGameMode — orchestrates waves, companion, scoring
- CombatComponent — shared health/damage component
- Projectile — energy projectile, overlap detection, ICombatDamageable support
- RobotEnemy — enemy character (type: Drone/Turret/Heavy)
- ChatWidget — original chat widget (backup)
- NeonPatrolHUD — health, wave, score display

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

- **Primary**: Groq API (`https://api.groq.com/openai/v1/chat/completions`)
- **Model**: llama-3.3-70b-versatile (fast, free, good at JSON commands)
- **API key**: loaded from `C:/Users/Kris/Kriisko-Studio/tools/.groq_key`
- **Fallback**: Qwen 0.8B on `http://127.0.0.1:11001` (local, needs llama-server)
- **Response format**: `{"say":"text", "cmd":"COMMAND PARAM"}`

## Audio Bridge (Voice Chat)

- **Server**: `python tools/audio_bridge.py` (port 7777)
- **STT**: Groq Whisper (`whisper-large-v3-turbo`)
- **TTS**: Windows SAPI (`System.Speech.Synthesis`)
- **Controls**: V key = push-to-talk (3 second recording)

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
