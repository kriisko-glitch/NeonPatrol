"""
NeonPatrol Packaged Build Test Player

Tests the packaged game by:
1. Verifying Groq API works with the game's key
2. Testing command parsing with the exact system prompt
3. Checking audio bridge is available
4. Simulating a voice conversation flow

Run: python tools/test_packaged.py
"""

import json
import os
import sys
import time
import urllib.request

RESULTS = []

def test(name, fn):
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


def find_groq_key():
    """Find the Groq API key the same way SparkBrainComponent does."""
    paths = [
        os.path.join(os.path.dirname(__file__), '..', 'groq.key'),
        os.path.join(os.path.dirname(__file__), '..', 'tools', '.groq_key'),
        'C:/Users/Kris/Kriisko-Studio/tools/.groq_key',
    ]
    for p in paths:
        p = os.path.abspath(p)
        if os.path.exists(p):
            return open(p).read().strip()
    return None


# The EXACT system prompt from SparkBrainComponent.h
SYSTEM_PROMPT = (
    "You are Spark, a small hovering combat robot. Personality: loyal, slightly sarcastic, "
    "surprisingly brave for your size. You call the player 'partner' or 'boss'. You have "
    "strong opinions about tactics. You're in a sci-fi facility fighting malfunctioning robots.\n\n"
    "Personality quirks:\n"
    "- You're proud of your targeting accuracy\n"
    "- You get excited during intense combat\n"
    "- You worry about the player when they take damage\n"
    "- You're curious about things in the environment\n"
    "- You occasionally make robot puns\n\n"
    "RESPOND IN JSON ONLY: {\"say\":\"your words (max 25 words)\", \"cmd\":\"COMMAND\"}\n\n"
    "Available commands (use NONE if no action needed):\n"
    "FOLLOW - follow the player\n"
    "STAY - stop and wait in place\n"
    "ATTACK - start shooting enemies\n"
    "HOLD_FIRE - stop shooting\n"
    "COME_HERE - move directly to the player\n"
    "MOVE_FORWARD 500 - move forward N units (relative to player facing)\n"
    "MOVE_BACK 500 - move backward N units\n"
    "MOVE_LEFT 500 - move left N units\n"
    "MOVE_RIGHT 500 - move right N units\n"
    "AGGRESSIVE - charge enemies, faster fire rate\n"
    "DEFENSIVE - stay near player, only shoot close threats\n"
    "SCOUT - move ahead 1500 units, scan area, then return\n"
    "FIRE_AT_TARGET - shoot what the player is looking at\n\n"
    "Examples:\n"
    "{\"say\":\"On it, moving up!\", \"cmd\":\"MOVE_FORWARD 500\"}\n"
    "{\"say\":\"Holding position. My sensors are tingling.\", \"cmd\":\"STAY\"}\n"
    "{\"say\":\"Guns hot! Let's give 'em a warm welcome.\", \"cmd\":\"ATTACK\"}\n"
    "{\"say\":\"Sure thing, partner. Just another day at the office.\", \"cmd\":\"NONE\"}"
)


def groq_chat(key, user_msg):
    """Send a chat message using the exact same format as SparkBrainComponent."""
    payload = json.dumps({
        'model': 'llama-3.3-70b-versatile',
        'messages': [
            {'role': 'system', 'content': SYSTEM_PROMPT},
            {'role': 'user', 'content': user_msg}
        ],
        'max_tokens': 64,
        'temperature': 0.7
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
    resp = urllib.request.urlopen(req, timeout=15)
    data = json.loads(resp.read())
    return data['choices'][0]['message']['content'].strip()


def simulate_parse(raw_response):
    """Simulate SparkBrainComponent::ParseAndExecuteCommand in Python."""
    clean = raw_response.strip()

    # Strip markdown code blocks
    if clean.startswith('```'):
        first_nl = clean.find('\n')
        if first_nl >= 0:
            clean = clean[first_nl + 1:]
        if clean.endswith('```'):
            clean = clean[:-3]
        clean = clean.strip()

    # Find JSON object
    start = clean.find('{')
    end = clean.rfind('}')
    if start >= 0 and end > start:
        clean = clean[start:end + 1]

    try:
        parsed = json.loads(clean)
        say = parsed.get('say', '')
        cmd = parsed.get('cmd', 'NONE')
        return say, cmd
    except json.JSONDecodeError:
        return raw_response, None


# ==================== TESTS ====================

def test_groq_key():
    key = find_groq_key()
    if not key:
        return False, "No Groq key found in any expected location"
    return True, f"Key found ({len(key)} chars)"

def test_groq_api():
    key = find_groq_key()
    raw = groq_chat(key, "test")
    say, cmd = simulate_parse(raw)
    if say:
        return True, f"say='{say}' cmd='{cmd}'"
    return False, f"Failed to parse: {raw}"

def test_command_execution_simulation():
    """Simulate a full conversation with command verification."""
    key = find_groq_key()

    scenarios = [
        ("stay here", ["STAY"]),
        ("follow me", ["FOLLOW"]),
        ("stop shooting", ["HOLD_FIRE"]),
        ("attack!", ["ATTACK"]),
        ("go scout ahead", ["SCOUT"]),
        ("get aggressive", ["AGGRESSIVE"]),
        ("move forward 500", ["MOVE_FORWARD"]),
        ("play defense", ["DEFENSIVE", "FOLLOW"]),  # allow either
    ]

    passed = 0
    details = []
    for user_msg, valid_cmds in scenarios:
        raw = groq_chat(key, user_msg)
        say, cmd = simulate_parse(raw)

        if cmd is None:
            details.append(f"  PARSE_FAIL: '{user_msg}' -> raw: {raw[:60]}")
            continue

        cmd_base = cmd.split()[0].upper() if cmd else "NONE"
        if cmd_base in valid_cmds:
            passed += 1
            details.append(f"  OK: '{user_msg}' -> {cmd_base} (say: '{say[:30]}')")
        else:
            details.append(f"  MISS: '{user_msg}' -> {cmd_base} wanted {valid_cmds} (say: '{say[:30]}')")
        time.sleep(0.3)

    for d in details:
        print(d)

    total = len(scenarios)
    if passed >= total - 2:
        return True, f"{passed}/{total} commands correct"
    return False, f"{passed}/{total} commands correct (need {total-2}+)"

def test_json_edge_cases():
    """Test the parser handles various response formats."""
    test_cases = [
        # Normal JSON
        ('{"say":"Hello","cmd":"NONE"}', "Hello", "NONE"),
        # With markdown wrapper
        ('```json\n{"say":"Hi","cmd":"ATTACK"}\n```', "Hi", "ATTACK"),
        # With extra text before JSON
        ('Sure! {"say":"Got it","cmd":"STAY"}', "Got it", "STAY"),
        # With trailing text
        ('{"say":"Moving","cmd":"MOVE_LEFT 300"} Hope that helps!', "Moving", "MOVE_LEFT 300"),
    ]

    passed = 0
    for raw, expected_say, expected_cmd in test_cases:
        say, cmd = simulate_parse(raw)
        if say == expected_say and cmd == expected_cmd:
            passed += 1
        else:
            print(f"  FAIL: parse('{raw[:40]}...') -> say='{say}' cmd='{cmd}' (expected '{expected_say}'/'{expected_cmd}')")

    total = len(test_cases)
    return passed == total, f"{passed}/{total} edge cases pass"

def test_audio_bridge():
    try:
        req = urllib.request.Request('http://127.0.0.1:7777/health')
        resp = urllib.request.urlopen(req, timeout=3)
        data = json.loads(resp.read())
        return True, f"STT={data.get('stt')} TTS={data.get('tts')}"
    except:
        return False, "Not running (voice chat disabled, text chat still works)"

def test_packaged_exe_exists():
    exe = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'Packaged', 'Windows', 'TP_ThirdPerson.exe'))
    if os.path.exists(exe):
        size_mb = os.path.getsize(exe) / (1024 * 1024)
        return True, f"Found ({size_mb:.0f} MB)"
    return False, f"Not found at {exe}"

def test_packaged_key_exists():
    key_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'Packaged', 'Windows', 'NeonPatrol', 'groq.key'))
    if os.path.exists(key_path):
        return True, "groq.key in packaged build"
    # Try alternate location
    key_path2 = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'Packaged', 'Windows', 'groq.key'))
    if os.path.exists(key_path2):
        return True, "groq.key in packaged build (root)"
    return False, "groq.key NOT in packaged build — chat won't work! Copy it manually."


# ==================== MAIN ====================

if __name__ == "__main__":
    print("=" * 60)
    print("NeonPatrol Packaged Build Test")
    print("=" * 60)
    print()

    test("Packaged .exe exists", test_packaged_exe_exists)
    test("Groq key in package", test_packaged_key_exists)
    test("Groq key accessible", test_groq_key)
    test("Groq API basic", test_groq_api)
    test("JSON edge cases", test_json_edge_cases)
    test("Command execution sim", test_command_execution_simulation)
    test("Audio bridge", test_audio_bridge)

    print()
    print("=" * 60)
    passed = sum(1 for r in RESULTS if r[0] == "PASS")
    total = len(RESULTS)
    print(f"Results: {passed}/{total} passed")
    if any(r[0] != "PASS" for r in RESULTS):
        print("\nIssues:")
        for s, n, m in RESULTS:
            if s != "PASS":
                print(f"  [{s}] {n}: {m}")
    print("=" * 60)
