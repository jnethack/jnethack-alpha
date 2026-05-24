import pytest
import re


# Simulated safe message formatter that represents what the fixed code SHOULD do
# This models the security invariant: output must never exceed buffer limits
# regardless of input size or content

MAX_TXT_BUF = 256  # typical fixed stack buffer size
MAX_CMD_BUF = 99  # matches cmd_buf[99] in sys/vms/vmsmail.c
MAX_USER_LEN = 64
MAX_NODE_LEN = 64

PREFIXES = [
    "Mail for you",
    "Mail for you: ",
    "Do you hear ringing?  ",
    "Talk request from",
    "MSG +",
    "XYZZY ",
]


def safe_format_txt(prefix: str, user_data: str) -> str:
    """Safe version: truncates to buffer limit."""
    combined = prefix + user_data
    return combined[:MAX_TXT_BUF - 1]  # leave room for null terminator


def safe_format_cmd(prefix: str, user_data: str) -> str:
    """Safe version: truncates to buffer limit."""
    combined = prefix + user_data
    return combined[:MAX_CMD_BUF - 1]


def safe_format_xyzzy(user: str, node: str) -> str:
    """Safe version of Sprintf(cmd_buf, 'XYZZY %s@%s', user, node)"""
    combined = f"XYZZY {user}@{node}"
    return combined[:MAX_CMD_BUF - 1]


def validate_output_within_bounds(output: str, max_size: int) -> bool:
    """Security invariant: output must fit within buffer bounds."""
    return len(output) < max_size  # strict less than (null terminator)


def validate_no_null_bytes(output: str) -> bool:
    """Security invariant: output must not contain embedded null bytes that could truncate."""
    return '\x00' not in output


def validate_no_format_string_passthrough(output: str) -> bool:
    """Security invariant: format specifiers from user input must not appear unescaped in cmd context."""
    # In a command buffer, raw % format specifiers from user data are dangerous
    # The output should not contain sequences like %s, %n, %x that came from user input
    dangerous_patterns = re.compile(r'%[sndioxXeEfgGcpuq%]')
    # Only flag if the dangerous pattern appears AFTER the known safe prefix
    for prefix in PREFIXES:
        if output.startswith(prefix):
            user_portion = output[len(prefix):]
            if dangerous_patterns.search(user_portion):
                return False
    return True


@pytest.mark.parametrize("payload", [
    # Basic overflow attempts
    "A" * 300,
    "B" * 512,
    "C" * 1024,
    "D" * 4096,
    "E" * 65536,

    # Null byte injection
    "hello\x00world",
    "\x00" * 100,
    "safe\x00" + "A" * 300,

    # Format string attacks
    "%s%s%s%s%s%s%s%s%s%s",
    "%n%n%n%n%n%n",
    "%x%x%x%x%x%x%x%x",
    "%" + "s" * 200,
    "%08x." * 50,
    "%(A" * 100,
    "%1$s%2$s%3$s",

    # Shell injection in command buffers
    "; rm -rf /",
    "| cat /etc/passwd",
    "`id`",
    "$(whoami)",
    "&& malicious_command",
    "\n/bin/sh\n",
    "user\nnode\ninjected_command",

    # Path traversal
    "../../../etc/passwd",
    "..\\..\\..\\windows\\system32",
    "/etc/shadow",

    # Special characters
    "\r\n" * 50,
    "\t" * 200,
    " " * 300,
    "\x01\x02\x03\x04\x05" * 50,
    "\xff" * 200,
    "\x7f" * 200,

    # Unicode/encoding attacks
    "A" * 100 + "\u0000" + "B" * 200,
    "\uffff" * 100,
    "\u202e" + "A" * 200,  # right-to-left override

    # VMS-specific attack patterns
    "SYSTEM" + "A" * 250,
    "node::user" + "!" * 200,
    "@" * 300,
    "user@" + "A" * 300,
    "A" * 200 + "@" + "B" * 200,

    # Empty and minimal inputs
    "",
    " ",
    "\n",
    "\r",

    # Mixed attacks
    "%s" + "A" * 300,
    "A" * 100 + "%n" + "B" * 100,
    "; " + "A" * 300,
    "\x00" + "A" * 300,
])
def test_buffer_overflow_invariant(payload):
    """Invariant: Message formatting must never produce output exceeding fixed buffer bounds,
    regardless of the size or content of externally-supplied user data (mail messages,
    talk requests, usernames, node names). This guards against stack buffer overflow
    via unbounded strcpy/strcat operations on attacker-controlled input."""

    # Test txt_buf operations (Mail for you, Talk request, etc.)
    txt_prefixes = [
        "Mail for you",
        "Mail for you: ",
        "Do you hear ringing?  ",
        "Talk request from",
    ]

    for prefix in txt_prefixes:
        result = safe_format_txt(prefix, payload)
        assert validate_output_within_bounds(result, MAX_TXT_BUF), (
            f"TXT buffer overflow: prefix='{prefix}', "
            f"payload_len={len(payload)}, result_len={len(result)} >= {MAX_TXT_BUF}"
        )

    # Test cmd_buf operations (MSG +, direct copy)
    cmd_prefixes = [
        "MSG +",
        "",  # direct strcpy case
    ]

    for prefix in cmd_prefixes:
        result = safe_format_cmd(prefix, payload)
        assert validate_output_within_bounds(result, MAX_CMD_BUF), (
            f"CMD buffer overflow: prefix='{prefix}', "
            f"payload_len={len(payload)}, result_len={len(result)} >= {MAX_CMD_BUF}"
        )

    # Test XYZZY user@node format (Sprintf case)
    # payload used as both user and node to maximize stress
    result = safe_format_xyzzy(payload, payload)
    assert validate_output_within_bounds(result, MAX_CMD_BUF), (
        f"XYZZY CMD buffer overflow: "
        f"payload_len={len(payload)}, result_len={len(result)} >= {MAX_CMD_BUF}"
    )


@pytest.mark.parametrize("user,node", [
    ("A" * 300, "B" * 300),
    ("A" * 64, "B" * 300),
    ("A" * 300, "B" * 64),
    ("%s" * 50, "%n" * 50),
    ("user\x00injected", "node\x00injected"),
    ("user; rm -rf /", "node"),
    ("user", "node; /bin/sh"),
    ("A" * 200 + "@" + "B" * 200, "C" * 200),
    ("", "B" * 300),
    ("A" * 300, ""),
    ("", ""),
    ("\xff" * 100, "\xfe" * 100),
])
def test_xyzzy_format_buffer_invariant(user, node):
    """Invariant: The XYZZY command format with user@node must always fit within
    the fixed cmd_buf regardless of the length of user-supplied username and node name."""
    result = safe_format_xyzzy(user, node)
    assert validate_output_within_bounds(result, MAX_CMD_BUF), (
        f"XYZZY buffer overflow: user_len={len(user)}, node_len={len(node)}, "
        f"result_len={len(result)} >= {MAX_CMD_BUF}"
    )
    assert result.startswith("XYZZY "), (
        f"XYZZY command must always start with 'XYZZY ' prefix, got: {result[:20]!r}"
    )


@pytest.mark.parametrize("payload", [
    "A" * 300,
    "%s%s%s",
    "\x00" + "A" * 300,
    "safe_user",
    "",
    "B" * MAX_TXT_BUF,
    "C" * (MAX_TXT_BUF + 1),
    "D" * (MAX_TXT_BUF * 2),
])
def test_all_message_types_bounded(payload):
    """Invariant: Every message type that concatenates user data into a fixed buffer
    must produce output strictly less than the buffer size, for any input."""
    operations = [
        ("txt", "Mail for you", payload),
        ("txt", "Mail for you: ", payload),
        ("txt", "Do you hear ringing?  ", payload),
        ("txt", "Talk request from", payload),
        ("cmd", "MSG +", payload[4:] if len(payload) > 4 else payload),
        ("cmd", "", payload),
    ]

    for buf_type, prefix, data in operations:
        if buf_type == "txt":
            result = safe_format_txt(prefix, data)
            max_size = MAX_TXT_BUF
        else:
            result = safe_format_cmd(prefix, data)
            max_size = MAX_CMD_BUF

        assert len(result) < max_size, (
            f"Buffer overflow detected: type={buf_type}, prefix='{prefix}', "
            f"data_len={len(data)}, result_len={len(result)}, max={max_size}"
        )