# -*- coding: utf-8 -*-
"""
EgLab 阶段一 (Command as Tool) —— DeepSeek 自然语言 → CommandManager 调用客户端

作用：作为独立于 C++ 引擎核心的轻量 AI 客户端（PLAN.md 中的"翻译官"角色），
     将用户自然语言解析为 CommandManager 已注册的 Command 及其参数，
     并输出等价于 CommandManager::call(EventId, EventPacket) 的协议载荷。

仅依赖 Python 3 标准库（无第三方依赖）。

用法:
    python deepseek_command_client.py --validate
    python deepseek_command_client.py --self-test
    python deepseek_command_client.py --prompt "打开网格导入面板"
    python deepseek_command_client.py                     # 交互式 REPL
    python deepseek_command_client.py --model deepseek-chat --api-key sk-xxx

API Key 通过环境变量 DEEPSEEK_API_KEY 或 --api-key 提供。
未提供 Key 时，--prompt / 交互模式会退化为内置的本地演示解析器（对应 PLAN 中
"本地小模型 + 云端大模型混合"中的本地轻量解析）。
"""

import argparse
import json
import os
import re
import sys
import urllib.error
import urllib.request

DEEPSEEK_API_URL = "https://api.deepseek.com/chat/completions"
DEFAULT_MODEL = "deepseek-chat"
SCHEMA_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "command_schemas.json")

SYSTEM_PROMPT = (
    "你是 EgLab 引擎开发平台的指令解析器。用户会用中文自然语言描述对引擎的操作意图。"
    "你的唯一职责是：把用户意图精确地转换为一个已注册 Command 的 tool call，"
    "并用 JSON 填充该 Command 要求的全部参数。规则：\n"
    "1. 只能调用 tools 中给出的函数，且每次只调用一个函数。\n"
    "2. 参数必须符合函数 schema：panelName 必须是枚举值；mode 必须是 0/1/2/3；"
    "ids 是用逗号或空格分隔的整数字符串（如 \"1, 2, 3\"）；路径使用绝对路径。\n"
    "3. 参数取值严格来自用户输入，不要臆造用户没提到的数值。\n"
    "4. 如果用户意图无法映射到任何函数（例如查询性能、切换渲染后端等尚未提供的命令），"
    "不要调用任何函数，直接回复一句简短的中文说明：该能力尚未注册为可用命令。"
)


def _ensure_utf8_console():
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8", errors="replace")
        except Exception:
            pass


def load_schema(path=SCHEMA_PATH):
    with open(path, "r", encoding="utf-8") as f:
        doc = json.load(f)
    commands = {c["name"]: c for c in doc["commands"]}
    return doc, commands


def build_tools(doc):
    """把 schema 文档转换为 OpenAI/DeepSeek function-calling 的 tools 数组。"""
    return [
        {
            "type": "function",
            "function": {
                "name": c["name"],
                "description": c["description"],
                "parameters": c["parameters"],
            },
        }
        for c in doc["commands"]
    ]


def call_deepseek(api_key, messages, tools, model=DEFAULT_MODEL, base_url=DEEPSEEK_API_URL):
    payload = {
        "model": model,
        "messages": messages,
        "tools": tools,
        "tool_choice": "auto",
        "temperature": 0.0,
        "stream": False,
    }
    request = urllib.request.Request(
        base_url,
        data=json.dumps(payload).encode("utf-8"),
        headers={
            "Content-Type": "application/json",
            "Authorization": "Bearer " + api_key,
        },
        method="POST",
    )
    try:
        with urllib.request.urlopen(request, timeout=60) as response:
            return json.loads(response.read().decode("utf-8"))
    except urllib.error.HTTPError as exc:
        detail = exc.read().decode("utf-8", errors="replace")
        raise RuntimeError(
            "DeepSeek API 返回 HTTP %s: %s" % (exc.code, detail)
        ) from exc


def extract_tool_calls(response):
    """从 DeepSeek 响应中提取 (tool_name, arguments_dict) 列表。"""
    choices = response.get("choices") or []
    if not choices:
        return []
    message = choices[0].get("message") or {}
    calls = message.get("tool_calls") or []
    result = []
    for call in calls:
        fn = call.get("function") or {}
        name = fn.get("name")
        raw = fn.get("arguments") or "{}"
        try:
            args = json.loads(raw)
        except (json.JSONDecodeError, TypeError):
            args = {}
        result.append((name, args))
    return result


def _coerce_value(value, schema):
    """按 JSON Schema 做最小限度的类型归一化，容忍 LLM 偶尔的类型偏差。"""
    kind = schema.get("type")
    if kind == "integer":
        if isinstance(value, bool):
            return int(value)
        if isinstance(value, int):
            return value
        if isinstance(value, float):
            return int(value)
        if isinstance(value, str):
            try:
                return int(value.strip())
            except ValueError:
                return None
        return None
    if kind == "string":
        if isinstance(value, str):
            return value
        if isinstance(value, (int, float)):
            return str(value)
        if isinstance(value, list):
            return ", ".join(str(v) for v in value)
        return None
    return value


def validate_and_normalize(command, args):
    """校验参数是否符合该命令的 parameters schema，返回 (ok, normalized, errors)。"""
    schema = command["parameters"]
    props = schema.get("properties", {})
    required = schema.get("required", [])
    normalized = {}
    errors = []

    for key in required:
        if key not in args:
            errors.append("缺少必填参数: %s" % key)

    for key, value in args.items():
        if key not in props:
            if not schema.get("additionalProperties", False):
                errors.append("未定义的参数: %s" % key)
            continue
        field = props[key]
        coerced = _coerce_value(value, field)
        if coerced is None:
            errors.append("参数 %s 的值 %r 不是有效的 %s" % (key, value, field.get("type")))
            continue
        if "enum" in field:
            enum_values = field["enum"]
            match = None
            if isinstance(coerced, str):
                for ev in enum_values:
                    if str(ev) == coerced or str(ev).lower() == coerced.lower():
                        match = ev
                        break
            elif coerced in enum_values:
                match = coerced
            if match is None:
                errors.append(
                    "参数 %s 的值 %r 不在枚举 %s 中" % (key, coerced, enum_values)
                )
                continue
            coerced = match
        if field.get("type") == "integer":
            if "minimum" in field and coerced < field["minimum"]:
                errors.append("参数 %s 小于最小值 %s" % (key, field["minimum"]))
            if "maximum" in field and coerced > field["maximum"]:
                errors.append("参数 %s 大于最大值 %s" % (key, field["maximum"]))
        if field.get("type") == "string":
            if "maxLength" in field and len(coerced) > field["maxLength"]:
                errors.append("参数 %s 超过最大长度 %s" % (key, field["maxLength"]))
            if "pattern" in field and not re.fullmatch(field["pattern"], coerced):
                errors.append("参数 %s 不符合格式约束: %r" % (key, coerced))
        normalized[key] = coerced

    return (len(errors) == 0, normalized, errors)


def serialize_payload(command, normalized):
    """产出 CommandManager 的协议载荷：eventId + EventPacket 字段键值。

    字段键名与 CommandManager::Impl::registerCommand() 中 _packetCreator
    读取的 EventPacket key 严格一致。
    """
    return {"eventId": command["eventId"], "name": command["name"], "payload": normalized}


def resolve_tool_call(commands, name, args):
    """把 (tool_name, arguments) 解析为协议载荷，并返回诊断信息。"""
    command = commands.get(name)
    if command is None:
        return None, "未知命令: %s（未在 command_schemas.json 中注册）" % name
    ok, normalized, errors = validate_and_normalize(command, args)
    if not ok:
        return None, "参数校验失败: " + "; ".join(errors)
    return serialize_payload(command, normalized), None


# ---------------------------------------------------------------------------
# 本地演示解析器（无 API Key 时的兜底，对应 PLAN 中的"本地小模型"占位实现）
# ---------------------------------------------------------------------------

_PANEL_KEYWORDS = [
    ("网格导入", "MeshImport"), ("导入面板", "MeshImport"), ("网格导入面板", "MeshImport"),
    ("相机", "CameraSetting"),
    ("数据树", "DataTree"),
    ("渲染设置", "MeshRenderSettings"), ("网格渲染", "MeshRenderSettings"),
    ("光照", "LightSetting"), ("灯光", "LightSetting"),
    ("背景", "BackGroundSetting"),
    ("元素可见性", "ElementVisibility"), ("单元可见性", "ElementVisibility"),
    ("日志", "MessageLog"),
    ("快捷键", "ShortcutSetting"),
    ("半边", "HEMesh"),
]


def local_resolve(text):
    """极简的关键词解析，仅用于演示/兜底，不追求完整 NLU。"""
    t = text.strip()
    if re.search(r"适应视图|居中显示|全部显示|fit", t, re.I):
        return "FitView", {}

    for keyword, panel in _PANEL_KEYWORDS:
        if keyword in t and re.search(r"打开|显示|切换|open|show", t, re.I):
            return "ShowPanel", {"panelName": panel}

    if re.search(r"导入|加载|import", t, re.I):
        path = re.search(r"[A-Za-z]:[\\/][^\s，。]+|[^\s，。]+\.(?:obj|stl|fbx|ply)", t, re.I)
        if path:
            return "MeshImport", {"fileDir": path.group(0)}

    m = re.search(r"删除.*?数据节点\s*(\d+)", t) or re.search(r"删除.*?节点\s*(\d+)", t)
    if m:
        return "MeshDelete", {"dataNodeId": int(m.group(1))}

    m = re.search(r"定位.*?节点\s*(\d+).*?单元\s*([\d\s,;，、]+)", t)
    if m:
        return "LocateElements", {"nodeId": int(m.group(1)), "ids": m.group(2).strip()}

    m = re.search(r"隐藏.*?节点\s*(\d+).*?单元\s*([\d\s,;，、]+)", t)
    if m:
        return "ElementsVisibility", {"nodeId": int(m.group(1)), "mode": 0,
                                      "ids": m.group(2).strip()}
    m = re.search(r"只显示.*?节点\s*(\d+).*?单元\s*([\d\s,;，、]+)", t)
    if m:
        return "ElementsVisibility", {"nodeId": int(m.group(1)), "mode": 2,
                                      "ids": m.group(2).strip()}
    m = re.search(r"隐藏.*?节点\s*(\d+).*?节点\s*([\d\s,;，、]+)", t)
    if m:
        return "NodeVisibility", {"nodeId": int(m.group(1)), "mode": 0,
                                  "ids": m.group(2).strip()}
    m = re.search(r"高亮.*?([\d\s,;，、]+)", t)
    if m:
        return "Highlight", {"ids": m.group(1).strip()}

    return None, None


SELF_TEST_CASES = [
    ("打开网格导入面板", "ShowPanel", {"panelName": "MeshImport"}),
    ("打开相机设置", "ShowPanel", {"panelName": "CameraSetting"}),
    ("适应视图", "FitView", {}),
    ("导入模型 C:/models/car.obj", "MeshImport", {"fileDir": "C:/models/car.obj"}),
    ("删除数据节点 42", "MeshDelete", {"dataNodeId": 42}),
    ("高亮数据节点 1, 2, 3", "Highlight", {"ids": "1, 2, 3"}),
    ("定位节点 7 的单元 100 200 300", "LocateElements", {"nodeId": 7, "ids": "100 200 300"}),
    ("隐藏节点 9 的单元 1, 2, 3", "ElementsVisibility", {"nodeId": 9, "mode": 0, "ids": "1, 2, 3"}),
    ("只显示节点 5 的单元 10 20", "ElementsVisibility", {"nodeId": 5, "mode": 2, "ids": "10 20"}),
]


def run_self_test(commands):
    passed = 0
    failed = 0
    for sentence, expected_name, expected_args in SELF_TEST_CASES:
        name, args = local_resolve(sentence)
        payload, err = resolve_tool_call(commands, name, args) if name else (None, "本地解析失败")
        ok = (
            payload is not None
            and payload["name"] == expected_name
            and payload["payload"] == expected_args
        )
        if ok:
            passed += 1
            print("  [PASS] %s -> %s %s" % (sentence, payload["name"], json.dumps(payload["payload"], ensure_ascii=False)))
        else:
            failed += 1
            print("  [FAIL] %s\n         expected %s %s\n         got %s (%s)"
                  % (sentence, expected_name, json.dumps(expected_args, ensure_ascii=False),
                     payload["name"] if payload else name, err))
    print("\n自测结果: %d 通过, %d 失败" % (passed, failed))
    return failed == 0


def render_result(payload, err):
    if err:
        print("✗ " + err)
        return
    print("✓ 已解析为 Command: %s" % payload["name"])
    print("  EventId         : %d" % payload["eventId"])
    print("  EventPacket 载荷 : %s" % json.dumps(payload["payload"], ensure_ascii=False))
    print("  等效 C++ 调用    : CommandManager::call(EventId::%s, packet);" % payload["name"])


def main():
    _ensure_utf8_console()
    parser = argparse.ArgumentParser(description="EgLab DeepSeek 命令解析客户端")
    parser.add_argument("--prompt", help="单次自然语言指令")
    parser.add_argument("--api-key", default=os.environ.get("DEEPSEEK_API_KEY", ""))
    parser.add_argument("--model", default=DEFAULT_MODEL)
    parser.add_argument("--schema", default=SCHEMA_PATH)
    parser.add_argument("--validate", action="store_true", help="仅校验并打印命令 schema")
    parser.add_argument("--self-test", action="store_true", help="离线自测（无需 API Key）")
    parser.add_argument("--allow-local-fallback", action="store_true",
                        help="无 API Key 时回退到本地演示解析器")
    args = parser.parse_args()

    doc, commands = load_schema(args.schema)
    tools = build_tools(doc)

    if args.validate:
        print("已加载 %d 个命令 schema:" % len(commands))
        for c in doc["commands"]:
            print("  - %-18s EventId=%d  [%s] %s" % (c["name"], c["eventId"], c["category"], c["summary"]))
        print("\nOpenAI tools 数组长度: %d" % len(tools))
        return 0

    if args.self_test:
        print("离线自测（本地演示解析器 + schema 校验）:\n")
        return 0 if run_self_test(commands) else 1

    def handle_sentence(sentence):
        if args.api_key:
            messages = [
                {"role": "system", "content": SYSTEM_PROMPT},
                {"role": "user", "content": sentence},
            ]
            try:
                response = call_deepseek(args.api_key, messages, tools, model=args.model)
            except Exception as exc:  # noqa: BLE001
                print("✗ DeepSeek 调用失败: %s" % exc)
                return 1
            calls = extract_tool_calls(response)
            if not calls:
                reply = ((response.get("choices") or [{}])[0].get("message") or {}).get("content", "")
                print("ℹ 未产生工具调用。模型回复: %s" % (reply or "(空)"))
                return 0
            exit_code = 0
            for name, call_args in calls:
                payload, err = resolve_tool_call(commands, name, call_args)
                render_result(payload, err)
                if err:
                    exit_code = 1
            return exit_code
        else:
            name, call_args = local_resolve(sentence)
            if name is None:
                print("✗ 未设置 DEEPSEEK_API_KEY，且本地演示解析器无法理解该指令。")
                print("  请设置环境变量 DEEPSEEK_API_KEY 或使用 --api-key 后重试。")
                return 1
            print("ℹ 未设置 DEEPSEEK_API_KEY，使用本地演示解析器（结果仅供流程演示）。")
            payload, err = resolve_tool_call(commands, name, call_args)
            render_result(payload, err)
            return 0 if not err else 1

    if args.prompt:
        return handle_sentence(args.prompt)

    print("EgLab DeepSeek 命令解析客户端（输入 exit 退出）")
    if not args.api_key:
        print("提示: 未设置 DEEPSEEK_API_KEY，将使用本地演示解析器。")
    while True:
        try:
            line = input(">> ").strip()
        except (EOFError, KeyboardInterrupt):
            print()
            return 0
        if not line:
            continue
        if line.lower() in ("exit", "quit", "q"):
            return 0
        handle_sentence(line)
    return 0


if __name__ == "__main__":
    sys.exit(main())
