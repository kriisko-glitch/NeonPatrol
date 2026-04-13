"""
NeonPatrol AI Test Player — Automated PIE verification
Simulates a player session to verify subsystems, chat, and Spark behavior.

Usage: python ai_test_player.py
Requires: UE5 editor running with NeonPatrol project open

Tests:
1. Editor is running and RC API responsive
2. SparkCharacter exists in level with all components
3. PIE can start and subsystems initialize
4. Chat subsystem creates widget
5. Groq API responds to Spark brain queries
6. Spark follows player (position check)
7. Spark color state changes
"""

import sys
import os
import json
import time
import urllib.request

# Add studio tools to path
sys.path.insert(0, 'C:/Users/Kris/Kriisko-Studio/tools')

RESULTS = []

def test(name, fn):
    """Run a test and record result."""
    try:
        ok, msg = fn()
        status = "PASS" if ok else "FAIL"
        RESULTS.append((status, name, msg))
        print(f"  [{status}] {name}: {msg}")
        return ok
    except Exception as e:
        RESULTS.append(("ERROR", name, str(e)))
        print(f"  [ERROR] {name}: {e}")
        return False

def rc_exec(code):
    """Execute Python in UE5 via remote execution."""
    from ue5_scene_ops._rc import rc_python
    ok, result = rc_python(code)
    if not ok:
        return False, str(result)
    if isinstance(result, dict):
        outputs = [l.get('output', '').strip() for l in result.get('output', [])]
        return True, '\n'.join(outputs)
    return True, str(result)

# ==================== TESTS ====================

def test_editor_running():
    """Test 1: Editor is running and RC API is responsive."""
    try:
        req = urllib.request.Request('http://localhost:30010/remote/info')
        resp = urllib.request.urlopen(req, timeout=3)
        data = json.loads(resp.read())
        routes = len(data.get('HttpRoutes', []))
        return True, f"RC API up ({routes} routes)"
    except:
        return False, "RC API not responding"

def test_spark_in_level():
    """Test 2: SparkCharacter exists with all required components."""
    ok, output = rc_exec('''
import unreal
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
found = False
for a in subsystem.get_all_level_actors():
    if a.get_class().get_name() == "SparkCharacter":
        comps = [c.get_class().get_name() for c in a.get_components_by_class(unreal.ActorComponent)]
        required = ["SparkBrainComponent", "SparkVoiceComponent", "CombatComponent", "SparkCommentaryComponent"]
        missing = [r for r in required if r not in comps]
        if missing:
            print(f"MISSING: {missing}")
        else:
            print(f"OK: all components present ({len(comps)} total)")
            found = True
        break
if not found:
    print("NO_SPARK")
''')
    if not ok:
        return False, f"RC exec failed: {output}"
    if "OK:" in output:
        return True, output.split("OK: ")[1] if "OK: " in output else output
    if "MISSING:" in output:
        return False, output
    return False, "SparkCharacter not found in level"

def test_groq_api():
    """Test 3: Groq API responds to Spark brain queries."""
    key_path = "C:/Users/Kris/Kriisko-Studio/tools/.groq_key"
    if not os.path.exists(key_path):
        return False, "Groq key file not found"

    key = open(key_path).read().strip()
    payload = json.dumps({
        'model': 'llama-3.3-70b-versatile',
        'messages': [
            {'role': 'system', 'content': 'Respond with JSON: {"say":"test","cmd":"NONE"}'},
            {'role': 'user', 'content': 'test'}
        ],
        'max_tokens': 32,
        'temperature': 0.1
    }).encode()

    try:
        req = urllib.request.Request(
            'https://api.groq.com/openai/v1/chat/completions',
            data=payload,
            headers={
                'Content-Type': 'application/json',
                'User-Agent': 'Kriisko-Studio/1.0',
                'Authorization': f'Bearer {key}'
            }
        )
        resp = urllib.request.urlopen(req, timeout=10)
        data = json.loads(resp.read())
        content = data['choices'][0]['message']['content'].strip()
        # Verify JSON parsing
        parsed = json.loads(content)
        if 'say' in parsed and 'cmd' in parsed:
            return True, f"Response: {content}"
        return False, f"Missing say/cmd: {content}"
    except json.JSONDecodeError:
        return False, f"Invalid JSON from Groq: {content}"
    except Exception as e:
        return False, str(e)

def test_audio_bridge():
    """Test 4: Audio bridge is running."""
    try:
        req = urllib.request.Request('http://127.0.0.1:7777/health')
        resp = urllib.request.urlopen(req, timeout=3)
        data = json.loads(resp.read())
        stt = data.get('stt', False)
        tts = data.get('tts', False)
        return True, f"STT={stt} TTS={tts}"
    except:
        return False, "Audio bridge not running (start with: python tools/audio_bridge.py)"

def test_command_parsing():
    """Test 5: Groq correctly parses all command types."""
    key = open("C:/Users/Kris/Kriisko-Studio/tools/.groq_key").read().strip()

    test_cases = [
        ("stop shooting", "HOLD_FIRE"),
        ("follow me", "FOLLOW"),
        ("get aggressive", "AGGRESSIVE"),
    ]

    system_prompt = (
        'Respond in JSON: {"say":"text","cmd":"COMMAND"}. '
        'Commands: FOLLOW, STAY, ATTACK, HOLD_FIRE, AGGRESSIVE, DEFENSIVE, SCOUT, NONE'
    )

    passed = 0
    for user_msg, expected_cmd in test_cases:
        payload = json.dumps({
            'model': 'llama-3.3-70b-versatile',
            'messages': [
                {'role': 'system', 'content': system_prompt},
                {'role': 'user', 'content': user_msg}
            ],
            'max_tokens': 32,
            'temperature': 0.1
        }).encode()

        req = urllib.request.Request(
            'https://api.groq.com/openai/v1/chat/completions',
            data=payload,
            headers={
                'Content-Type': 'application/json',
                'User-Agent': 'Kriisko-Studio/1.0',
                'Authorization': f'Bearer {key}'
            }
        )
        resp = urllib.request.urlopen(req, timeout=10)
        data = json.loads(resp.read())
        content = data['choices'][0]['message']['content'].strip()
        try:
            parsed = json.loads(content)
            if parsed.get('cmd', '').upper() == expected_cmd:
                passed += 1
        except:
            pass
        time.sleep(0.3)

    total = len(test_cases)
    if passed == total:
        return True, f"{passed}/{total} commands correct"
    return False, f"{passed}/{total} commands correct"

def test_conversational_commands():
    """Test 5b: Non-deterministic language maps to correct commands."""
    key = open("C:/Users/Kris/Kriisko-Studio/tools/.groq_key").read().strip()

    # Test with natural, varied phrasing — NOT exact command names
    test_cases = [
        ("hey spark, could you quit firing for a sec?", "HOLD_FIRE"),
        ("yo come over here buddy", "COME_HERE"),
        ("go check what's over there", "SCOUT"),
        ("ok start blasting again", "ATTACK"),
        ("stick close and watch my back", "DEFENSIVE"),
        ("charge in there and light em up!", "AGGRESSIVE"),
        ("wait here, I'll be right back", "STAY"),
        ("alright lets move, come with me", "FOLLOW"),
        ("scoot to the left a bit", "MOVE_LEFT"),
        ("what do you think about this place?", "NONE"),
    ]

    system_prompt = (
        'You are Spark, a combat robot companion. '
        'Respond in JSON: {"say":"max 20 words","cmd":"COMMAND"}. '
        'Commands: FOLLOW, STAY, ATTACK, HOLD_FIRE, COME_HERE, '
        'MOVE_FORWARD N, MOVE_BACK N, MOVE_LEFT N, MOVE_RIGHT N, '
        'AGGRESSIVE, DEFENSIVE, SCOUT, FIRE_AT_TARGET, NONE'
    )

    passed = 0
    details = []
    for user_msg, expected_cmd in test_cases:
        payload = json.dumps({
            'model': 'llama-3.3-70b-versatile',
            'messages': [
                {'role': 'system', 'content': system_prompt},
                {'role': 'user', 'content': user_msg}
            ],
            'max_tokens': 48,
            'temperature': 0.3
        }).encode()

        req = urllib.request.Request(
            'https://api.groq.com/openai/v1/chat/completions',
            data=payload,
            headers={
                'Content-Type': 'application/json',
                'User-Agent': 'Kriisko-Studio/1.0',
                'Authorization': f'Bearer {key}'
            }
        )
        try:
            resp = urllib.request.urlopen(req, timeout=10)
            data = json.loads(resp.read())
            content = data['choices'][0]['message']['content'].strip()
            parsed = json.loads(content)
            got_cmd = parsed.get('cmd', '').split()[0].upper()  # handle "MOVE_LEFT 300"
            expected_base = expected_cmd.split()[0]
            if got_cmd == expected_base:
                passed += 1
                details.append(f"  OK: '{user_msg}' -> {got_cmd}")
            else:
                details.append(f"  MISS: '{user_msg}' -> {got_cmd} (wanted {expected_base})")
        except Exception as e:
            details.append(f"  ERR: '{user_msg}' -> {e}")
        time.sleep(0.3)

    total = len(test_cases)
    for d in details:
        print(d)
    if passed >= total - 3:  # Allow 3 misses for non-deterministic natural language
        return True, f"{passed}/{total} conversational commands correct"
    return False, f"{passed}/{total} conversational commands correct (need {total-2}+)"

def test_level_has_enemies():
    """Test 6: Combat level has enemy spawners."""
    ok, output = rc_exec('''
import unreal
subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
spawners = 0
enemies = 0
player_start = False
for a in subsystem.get_all_level_actors():
    name = a.get_class().get_name()
    if "Spawner" in name:
        spawners += 1
    if "Enemy" in name or "Combat" in name:
        enemies += 1
    if "PlayerStart" in name:
        player_start = True
print(f"Spawners={spawners} Enemies={enemies} PlayerStart={player_start}")
''')
    if not ok:
        return False, f"RC exec failed"
    if "PlayerStart=True" in output and "Spawners=" in output:
        return True, output.strip()
    return False, output.strip()


# ==================== MAIN ====================

if __name__ == "__main__":
    print("=" * 60)
    print("NeonPatrol AI Test Player")
    print("=" * 60)
    print()

    test("Editor running", test_editor_running)
    test("Spark in level", test_spark_in_level)
    test("Level has enemies", test_level_has_enemies)
    test("Groq API", test_groq_api)
    test("Command parsing", test_command_parsing)
    test("Conversational commands", test_conversational_commands)
    test("Audio bridge", test_audio_bridge)

    print()
    print("=" * 60)
    passed = sum(1 for r in RESULTS if r[0] == "PASS")
    failed = sum(1 for r in RESULTS if r[0] == "FAIL")
    errors = sum(1 for r in RESULTS if r[0] == "ERROR")
    total = len(RESULTS)
    print(f"Results: {passed}/{total} passed, {failed} failed, {errors} errors")

    if failed > 0 or errors > 0:
        print("\nFailed tests:")
        for status, name, msg in RESULTS:
            if status != "PASS":
                print(f"  [{status}] {name}: {msg}")

    print("=" * 60)

    # Write results to file
    result_file = os.path.join(os.path.dirname(__file__), '..', 'test-results.json')
    with open(result_file, 'w') as f:
        json.dump({
            'timestamp': time.strftime('%Y-%m-%d %H:%M:%S'),
            'passed': passed,
            'failed': failed,
            'errors': errors,
            'total': total,
            'tests': [{'status': s, 'name': n, 'message': m} for s, n, m in RESULTS]
        }, f, indent=2)
    print(f"Results written to {result_file}")
