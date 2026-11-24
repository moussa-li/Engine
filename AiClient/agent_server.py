# -*- coding: utf-8 -*-
"""
EgLab-Agent —— 独立 Python Agent 进程（AI↔EnginePlatform 链路中间层）

职责（对应架构图中的 "EgLab-Agent" 层）:
  * 会话管理与上下文维护
  * 调用 DeepSeek（function-calling + 流式）解析自然语言意图
  * 工具调用循环（对话即操作：多步调用 CreateMesh/SetCamera/GetLog 等引擎工具）
  * 通过 CommandBridge 把工具调用转发给 EnginePlatform 内嵌的 MCP Server
  * 通过 HTTP + SSE 把流式文本与工具调用事件推送给 AI Dialog UI

零第三方依赖（仅 Python 3 标准库）。

用法:
    python agent_server.py                    # 默认 native：直连运行中 EnginePlatform（真实命令）
    python agent_server.py --bridge stub      # 无引擎时用 stub 后端演示
    python agent_server.py --bridge mcp --mcp-pipe eglab_mcp   # 连接 C++ 内嵌 MCP Server（P1）

    UI 默认地址: http://127.0.0.1:8765
"""

import argparse
import ctypes
import json
import os
import queue
import re
import struct
import sys
import threading
import time
import urllib.error
import urllib.parse
import urllib.request
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

try:
    import winreg as _winreg  # Windows 注册表（读取 EnginePlatform 进程 PID）
except ImportError:  # 非 Windows 环境
    _winreg = None

DEEPSEEK_API_URL = "https://api.deepseek.com/chat/completions"
DEFAULT_MODEL = "deepseek-chat"
DEFAULT_HOST = "127.0.0.1"
DEFAULT_HTTP_PORT = 8765
MAX_AGENT_STEPS = 8

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
MCP_TOOLS_PATH = os.path.join(BASE_DIR, "mcp_tools.json")
COMMAND_SCHEMAS_PATH = os.path.join(BASE_DIR, "command_schemas.json")
UI_INDEX_PATH = os.path.join(BASE_DIR, "ui", "index.html")

SYSTEM_PROMPT = (
    "你是 EgLab 引擎开发平台的研发调度中枢（Dev-Commander）。用户用中文自然语言描述"
    "对引擎的操作或提问。你可以调用引擎工具来完成用户意图。规则：\n"
    "1. 只能调用 tools 中给出的函数；必要时可分多步调用（例如先创建网格再设置相机）。\n"
    "2. 参数必须符合函数 schema，取值严格来自用户输入，不要臆造用户没提到的数值。\n"
    "3. 每次工具调用后，根据返回结果决定是否继续调用或给出最终回答。\n"
    "4. 如果意图无法用现有工具完成（例如修改物理参数、导出文件等未提供的工具），"
    "不要调用任何函数，直接说明该能力尚未提供。\n"
    "5. 最终回答使用简洁的中文，并简要说明你执行了哪些工具调用及结果。"
)


def _ensure_utf8_console():
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8", errors="replace")
        except Exception:
            pass


def read_engine_pid():
    """读取 EnginePlatform 写入注册表的进程 PID（PidRegistry）。

    对应 HKCU\\Software\\EngineLab\\CURRENT_ENGINELAB_PROCESSID (REG_DWORD)。
    未找到或读取失败时返回 0。
    """
    if _winreg is None:
        return 0
    try:
        key = _winreg.OpenKey(_winreg.HKEY_CURRENT_USER, "Software\\EngineLab",
                              0, _winreg.KEY_QUERY_VALUE)
        try:
            value, _ = _winreg.QueryValueEx(key, "CURRENT_ENGINELAB_PROCESSID")
        finally:
            _winreg.CloseKey(key)
        return int(value)
    except OSError:
        return 0


def is_pid_running(pid):
    """通过 OpenProcess 判断 PID 对应的进程是否仍在运行（Windows）。"""
    if pid <= 0:
        return False
    try:
        kernel32 = ctypes.windll.kernel32
        process_query_limited_information = 0x1000
        handle = kernel32.OpenProcess(process_query_limited_information, False, pid)
        if not handle:
            return False
        kernel32.CloseHandle(handle)
        return True
    except Exception:  # noqa: BLE001
        return False


# ---------------------------------------------------------------------------
# 原生命令桥：直接通过共享内存环形队列调用运行中 EnginePlatform 的 CommandManager
# ---------------------------------------------------------------------------

RING_BUFFER_SIZE = 1024
RING_BUFFER_MASK = RING_BUFFER_SIZE - 1
COMMON_MAX_PAYLOAD = 65536
SLOT_HEADER_SIZE = 24  # MessageHeader: u64 ts + u32 state + u32 channelId + u32 size + u32 reserved
SLOT_STRIDE = SLOT_HEADER_SIZE + COMMON_MAX_PAYLOAD  # 65560
CONTROL_BLOCK_SIZE = 3 * 64 + RING_BUFFER_SIZE * SLOT_HEADER_SIZE  # 192 + 24576 = 24768
SHM_SIZE = CONTROL_BLOCK_SIZE + RING_BUFFER_SIZE * SLOT_STRIDE

FILE_MAP_ALL_ACCESS = 0x000F001F
SLOT_EMPTY, SLOT_WRITING, SLOT_READY, SLOT_READING = 0, 1, 2, 3


def _setup_kernel32():
    k = ctypes.windll.kernel32
    k.OpenFileMappingA.restype = ctypes.c_void_p
    k.OpenFileMappingA.argtypes = [ctypes.c_uint32, ctypes.c_int, ctypes.c_char_p]
    k.MapViewOfFile.restype = ctypes.c_void_p
    k.MapViewOfFile.argtypes = [ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint32,
                                ctypes.c_uint32, ctypes.c_size_t]
    k.UnmapViewOfFile.restype = ctypes.c_int
    k.UnmapViewOfFile.argtypes = [ctypes.c_void_p]
    k.CloseHandle.restype = ctypes.c_int
    k.CloseHandle.argtypes = [ctypes.c_void_p]
    k.CreateToolhelp32Snapshot.restype = ctypes.c_void_p
    k.CreateToolhelp32Snapshot.argtypes = [ctypes.c_uint32, ctypes.c_uint32]
    k.Process32FirstW.restype = ctypes.c_int
    k.Process32FirstW.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    k.Process32NextW.restype = ctypes.c_int
    k.Process32NextW.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    k.CreateMutexW.restype = ctypes.c_void_p
    k.CreateMutexW.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_wchar_p]
    k.WaitForSingleObject.restype = ctypes.c_uint32
    k.WaitForSingleObject.argtypes = [ctypes.c_void_p, ctypes.c_uint32]
    k.ReleaseMutex.restype = ctypes.c_int
    k.ReleaseMutex.argtypes = [ctypes.c_void_p]
    return k


def parse_ids(text):
    text = (text or "").replace("，", ",").replace("、", ",")
    ids = []
    for token in re.split(r"[\s,;]+", text.strip()):
        if token.isdigit():
            ids.append(int(token))
    return ids


def is_number(value):
    """判断 value 是否为数值（排除 bool）。"""
    return isinstance(value, (int, float)) and not isinstance(value, bool)


def parse_vec3(value):
    """把 [x,y,z] 或 "x,y,z" 解析为 (x, y, z) 三个 float；失败返回 None。"""
    if value is None:
        return None
    if isinstance(value, (list, tuple)):
        tokens = list(value)
    elif isinstance(value, str):
        tokens = re.split(r"[\s,;]+", value.strip().replace("，", ","))
        tokens = [t for t in tokens if t != ""]
    else:
        return None
    if len(tokens) < 3:
        return None
    try:
        return float(tokens[0]), float(tokens[1]), float(tokens[2])
    except (ValueError, TypeError):
        return None


def parse_vec4(value):
    """把 [r,g,b,a] 或 "r,g,b,a" 解析为四个 float；失败返回 None。"""
    if value is None:
        return None
    if isinstance(value, (list, tuple)):
        tokens = list(value)
    elif isinstance(value, str):
        tokens = re.split(r"[\s,;]+", value.strip().replace("，", ","))
        tokens = [t for t in tokens if t != ""]
    else:
        return None
    if len(tokens) < 4:
        return None
    try:
        return float(tokens[0]), float(tokens[1]), float(tokens[2]), float(tokens[3])
    except (ValueError, TypeError):
        return None


def find_engine_pids():
    """枚举当前运行的 EnginePlatform.exe 进程 PID（注册表 PID 失效时的回退）。"""
    try:
        k = _setup_kernel32()
        INVALID_HANDLE_VALUE = ctypes.c_void_p(-1).value
        class PROCESSENTRY32W(ctypes.Structure):
            _fields_ = [
                ("dwSize", ctypes.c_uint32),
                ("cntUsage", ctypes.c_uint32),
                ("th32ProcessID", ctypes.c_uint32),
                ("th32DefaultHeapID", ctypes.c_size_t),
                ("th32ModuleID", ctypes.c_uint32),
                ("cntThreads", ctypes.c_uint32),
                ("th32ParentProcessID", ctypes.c_uint32),
                ("pcPriClassBase", ctypes.c_long),
                ("dwFlags", ctypes.c_uint32),
                ("szExeFile", ctypes.c_wchar * 260),
            ]
        snap = k.CreateToolhelp32Snapshot(0x00000002, 0)  # TH32CS_SNAPPROCESS
        if snap == INVALID_HANDLE_VALUE:
            return []
        entry = PROCESSENTRY32W()
        entry.dwSize = ctypes.sizeof(PROCESSENTRY32W)
        result = []
        if k.Process32FirstW(snap, ctypes.byref(entry)):
            while True:
                if entry.szExeFile.lower() == "engineplatform.exe":
                    result.append(int(entry.th32ProcessID))
                if not k.Process32NextW(snap, ctypes.byref(entry)):
                    break
        k.CloseHandle(snap)
        return result
    except Exception:  # noqa: BLE001
        return []


def resolve_engine_pid():
    """确定目标 EnginePlatform 的 PID：优先注册表，失效时回退到进程枚举。"""
    pid = read_engine_pid()
    if pid > 0 and is_pid_running(pid):
        return pid
    pids = find_engine_pids()
    return pids[0] if pids else 0


class NativeCommandBridge:
    """真实命令桥：通过共享内存环形队列调用运行中 EnginePlatform 的 CommandManager。

    与 CommandManager.cpp / CommandTransport.cpp 的跨进程协议一致：
      - 打开文件映射 "Command Memory-<pid>"（Common::MemoryQueue）
      - 以 channelId=EventId 写入 MessageHeader + 载荷（PacketPOD 或可变长 wire 格式）
    命令执行为 fire-and-forget（无返回值），执行结果需通过日志/其它通道获取。
    """

    def __init__(self):
        self._k = _setup_kernel32()
        self._pid = 0
        self._base = None
        self._handle = None
        self._mutex = None

    def _disconnect(self):
        if self._base:
            self._k.UnmapViewOfFile(self._base)
            self._base = None
        if self._handle:
            self._k.CloseHandle(self._handle)
            self._handle = None
        self._pid = 0

    def _connect(self, pid):
        self._disconnect()
        name = ("Command Memory-%d" % pid).encode("ascii")
        handle = self._k.OpenFileMappingA(FILE_MAP_ALL_ACCESS, False, name)
        if not handle:
            return False
        base = self._k.MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, SHM_SIZE)
        if not base:
            self._k.CloseHandle(handle)
            return False
        self._handle = handle
        self._base = base
        self._pid = pid
        return True

    @staticmethod
    def _read_u64(addr):
        return ctypes.c_uint64.from_address(addr).value

    @staticmethod
    def _write_u64(addr, value):
        ctypes.c_uint64.from_address(addr).value = value

    @staticmethod
    def _read_u32(addr):
        return ctypes.c_uint32.from_address(addr).value

    @staticmethod
    def _write_u32(addr, value):
        ctypes.c_uint32.from_address(addr).value = value

    def _acquire_mutex(self):
        if self._mutex is None:
            self._mutex = self._k.CreateMutexW(None, False, "Local\\EgLabAgentCmdQueue")
        if not self._mutex:
            return False
        return self._k.WaitForSingleObject(self._mutex, 0xFFFFFFFF) == 0  # WAIT_OBJECT_0

    def _release_mutex(self):
        if self._mutex:
            self._k.ReleaseMutex(self._mutex)

    def _push(self, channel_id, payload):
        if not payload:
            payload = b""
        if len(payload) > COMMON_MAX_PAYLOAD:
            return False
        if not self._acquire_mutex():
            return False
        try:
            write = self._read_u64(self._base + 0)
            read = self._read_u64(self._base + 64)
            if (write - read + 1) > RING_BUFFER_SIZE:
                return False
            slot = self._base + CONTROL_BLOCK_SIZE + (write & RING_BUFFER_MASK) * SLOT_STRIDE
            deadline = time.time() + 2.0
            while self._read_u32(slot + 8) != SLOT_EMPTY:
                if time.time() > deadline:
                    return False
                time.sleep(0)
            self._write_u32(slot + 8, SLOT_WRITING)
            self._write_u64(slot + 0, time.time_ns())
            self._write_u32(slot + 12, channel_id)
            self._write_u32(slot + 16, len(payload))
            self._write_u32(slot + 20, 0)
            if payload:
                ctypes.memmove(slot + 24, payload, len(payload))
            self._write_u32(slot + 8, SLOT_READY)
            # 发布：最后递增 writeIdx，消费方才会看到该槽。
            # 单生产者（AI Agent）下普通写即可；x86 的 store 顺序保证先见数据、后见写指针。
            self._write_u64(self._base + 0, write + 1)
            return True
        finally:
            self._release_mutex()

    def _read_result(self, pid, event_id, timeout=3.0):
        """读取同步查询回传结果（EgLab MeshDataResult-<pid> 队列）。

        引擎把查询结果 push 到该队列，channelId = 对应命令的 eventId。本方法轮询消费：
        跳过与 event_id 不匹配的槽（消费并丢弃），直到取到目标槽或超时。
        支持 Common::MemoryQueue 的大载荷分片（marker=0x46524147 'FRAG'）自动重组。
        返回 (bytes|None, error_str|None)。
        """
        FRAGMENT_MARKER = 0x46524147
        FRAGMENT_HEADER_SIZE = 16  # u64 messageId + u32 totalSize + u32 offset
        name = ("EgLab MeshDataResult-%d" % pid).encode("ascii")
        handle = self._k.OpenFileMappingA(FILE_MAP_ALL_ACCESS, False, name)
        if not handle:
            return None, "无法打开网格数据结果队列（共享内存 EgLab MeshDataResult-%d）" % pid
        try:
            base = self._k.MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, SHM_SIZE)
            if not base:
                return None, "无法映射网格数据结果队列内存"
            try:
                deadline = time.time() + timeout
                fragment_message_id = None
                fragment_total_size = 0
                fragment_received = 0
                fragment_buffer = None
                fragment_channel = 0
                while True:
                    write = self._read_u64(base + 0)
                    read = self._read_u64(base + 64)
                    if read < write:
                        slot = base + CONTROL_BLOCK_SIZE + (read & RING_BUFFER_MASK) * SLOT_STRIDE
                        state = self._read_u32(slot + 8)
                        if state == SLOT_READY:
                            channel = self._read_u32(slot + 12)
                            size = self._read_u32(slot + 16)
                            marker = self._read_u32(slot + 20)
                            # 消费侧推进：先取数据，再置 EMPTY + readIdx++（普通写即可）
                            data_len = min(size, COMMON_MAX_PAYLOAD)
                            buf = ctypes.create_string_buffer(data_len)
                            if data_len:
                                ctypes.memmove(buf, slot + 24, data_len)
                            self._write_u32(slot + 8, SLOT_EMPTY)
                            self._write_u64(base + 64, read + 1)

                            if marker == FRAGMENT_MARKER:
                                if data_len < FRAGMENT_HEADER_SIZE:
                                    continue
                                message_id, total_size, offset = struct.unpack_from(
                                    "<QII", buf.raw, 0)
                                chunk = bytes(buf.raw[FRAGMENT_HEADER_SIZE:data_len])
                                if offset == 0:
                                    fragment_message_id = message_id
                                    fragment_total_size = total_size
                                    fragment_channel = channel
                                    fragment_buffer = bytearray(total_size)
                                    fragment_received = 0
                                if (message_id != fragment_message_id
                                        or total_size != fragment_total_size
                                        or offset != fragment_received
                                        or channel != fragment_channel):
                                    fragment_message_id = None
                                    continue
                                fragment_buffer[offset:offset + len(chunk)] = chunk
                                fragment_received += len(chunk)
                                if fragment_received == fragment_total_size:
                                    fragment_message_id = None
                                    if fragment_channel == event_id:
                                        return bytes(fragment_buffer), None
                                continue

                            if channel == event_id:
                                return bytes(buf.raw[:data_len]), None
                            continue
                    if time.time() > deadline:
                        return None, "查询结果超时（eventId=%d）" % event_id
                    time.sleep(0.005)
            finally:
                self._k.UnmapViewOfFile(base)
        finally:
            self._k.CloseHandle(handle)

    @staticmethod
    def _parse_mesh_result(data):
        """解析 GetMeshDataResult（64B header + 256×200B entries，小端）。"""
        if len(data) < 64:
            return {"ok": False, "error": "GetMeshData 结果数据不完整"}
        magic, version, status, flags, mesh_count, total_node, total_elem, bmin_x, bmin_y, bmin_z, bmax_x, bmax_y, bmax_z = struct.unpack_from(
            "<IIIII4xQQ3f3f", data, 0)
        if magic != 0x4D534844:
            return {"ok": False, "error": "GetMeshData 结果 magic 校验失败: 0x%08X" % magic}
        if version != 1:
            return {"ok": False, "error": "GetMeshData 结果 version 不支持: %d" % version}
        if status != 0:
            return {"ok": False, "error": "目标数据节点不存在或非 MeshData（status=%d）" % status}
        count = min(mesh_count, 256)
        meshes = []
        for i in range(count):
            off = 64 + i * 200
            if off + 196 > len(data):
                break
            data_node_id, name = struct.unpack_from("<Q64s", data, off)
            node_count, elem_count = struct.unpack_from("<QQ", data, off + 72)
            bmin = struct.unpack_from("<3f", data, off + 88)
            bmax = struct.unpack_from("<3f", data, off + 100)
            elem_type_count = struct.unpack_from("<I", data, off + 112)[0]
            elem_type_ids = struct.unpack_from("<10I", data, off + 116)
            elem_type_counts = struct.unpack_from("<10I", data, off + 156)
            meshes.append({
                "dataNodeId": int(data_node_id),
                "name": name.rstrip(b"\x00").decode("utf-8", errors="replace"),
                "nodeCount": int(node_count),
                "elemCount": int(elem_count),
                "bboxMin": list(bmin),
                "bboxMax": list(bmax),
                "elemTypeCount": int(elem_type_count),
                "elemTypes": [
                    {"id": int(elem_type_ids[j]), "count": int(elem_type_counts[j])}
                    for j in range(min(elem_type_count, 10))
                ],
            })
        return {
            "ok": True,
            "result": {
                "status": "ok",
                "meshCount": count,
                "truncated": bool(flags & 1),
                "totalNodeCount": int(total_node),
                "totalElemCount": int(total_elem),
                "totalBBox": {"min": [bmin_x, bmin_y, bmin_z],
                              "max": [bmax_x, bmax_y, bmax_z]},
                "meshes": meshes,
            },
        }

    @staticmethod
    def _parse_query_header(data):
        """解析精确查询结果公共头（32B：magic/version/status/total/offset/returned/2×reserved）。"""
        if len(data) < 32:
            return None, {"ok": False, "error": "查询结果数据不完整"}
        magic, version, status, total, offset, returned, _, _ = struct.unpack_from("<IIIIIIII", data, 0)
        if magic != 0x4D534844:
            return None, {"ok": False, "error": "查询结果 magic 校验失败: 0x%08X" % magic}
        if version != 1:
            return None, {"ok": False, "error": "查询结果 version 不支持: %d" % version}
        if status != 0:
            return None, {"ok": False,
                          "error": "网格不存在或非 MeshData（status=%d）" % status}
        header = {"status": "ok", "total": int(total), "offset": int(offset),
                  "returned": int(returned)}
        return header, None

    @staticmethod
    def _parse_list_data_tree(data):
        header, err = NativeCommandBridge._parse_query_header(data)
        if err:
            return err
        entries = []
        for i in range(header["returned"]):
            off = 32 + i * 96
            if off + 96 > len(data):
                break
            node_id, name = struct.unpack_from("<Q64s", data, off)
            kind, _, node_count, elem_count = struct.unpack_from("<IIQQ", data, off + 72)
            entries.append({
                "dataNodeId": int(node_id),
                "name": name.rstrip(b"\x00").decode("utf-8", errors="replace"),
                "kind": int(kind),  # 1=MeshData, 0=其它 DataNode
                "nodeCount": int(node_count),
                "elemCount": int(elem_count),
            })
        return {"ok": True, "result": dict(header, nodes=entries)}

    @staticmethod
    def _parse_mesh_ids(data):
        header, err = NativeCommandBridge._parse_query_header(data)
        if err:
            return err
        ids = []
        for i in range(header["returned"]):
            off = 32 + i * 4
            if off + 4 > len(data):
                break
            ids.append(int(struct.unpack_from("<I", data, off)[0]))
        return {"ok": True, "result": dict(header, ids=ids)}

    @staticmethod
    def _parse_nodes_xyz(data):
        header, err = NativeCommandBridge._parse_query_header(data)
        if err:
            return err
        nodes = []
        for i in range(header["returned"]):
            off = 32 + i * 16
            if off + 16 > len(data):
                break
            node_id, x, y, z = struct.unpack_from("<I3f", data, off)
            nodes.append({"id": int(node_id), "xyz": [round(float(x), 6),
                                                       round(float(y), 6),
                                                       round(float(z), 6)]})
        return {"ok": True, "result": dict(header, nodes=nodes)}

    @staticmethod
    def _parse_elements_topo(data):
        header, err = NativeCommandBridge._parse_query_header(data)
        if err:
            return err
        elements = []
        for i in range(header["returned"]):
            off = 32 + i * 92
            if off + 92 > len(data):
                break
            elem_id, elem_type, node_count = struct.unpack_from("<III", data, off)
            node_ids = struct.unpack_from("<20I", data, off + 12)
            elements.append({
                "id": int(elem_id),
                "type": int(elem_type),
                "nodeCount": int(node_count),
                "nodeIds": [int(n) for n in node_ids[:node_count]],
            })
        return {"ok": True, "result": dict(header, elements=elements)}

    @staticmethod
    def _parse_geometry_header(data):
        """解析几何查询结果公共头（32B，magic=0x4D4F4547 'GEOM'）。"""
        if len(data) < 32:
            return None, {"ok": False, "error": "几何查询结果数据不完整"}
        magic, version, status, total, offset, returned, _, _ = struct.unpack_from(
            "<IIIIIIII", data, 0)
        if magic != 0x4D4F4547:
            return None, {"ok": False, "error": "几何结果 magic 校验失败: 0x%08X" % magic}
        if version != 1:
            return None, {"ok": False, "error": "几何结果 version 不支持: %d" % version}
        status_msgs = {
            1: "数据节点不存在或非 GeometryData",
            2: "特征/elemId 不存在或参数非法",
            3: "几何/Tessellation 不可用",
            4: "featureType 非法",
        }
        if status != 0:
            return None, {"ok": False,
                          "error": "%s（status=%d）" % (status_msgs.get(status, "未知错误"), status)}
        header = {"status": "ok", "total": int(total), "offset": int(offset),
                  "returned": int(returned)}
        return header, None

    @staticmethod
    def _parse_geometry_entry(data, off):
        """解析单个 GeometryFeatureEntry（152B）。"""
        feature_id, feature_type, geometry_type, parent_count, child_count, elem_id = \
            struct.unpack_from("<IIIIII", data, off)
        length, area, volume, x, y, z = struct.unpack_from("<6d", data, off + 24)
        bbox_min = struct.unpack_from("<3f", data, off + 72)
        bbox_max = struct.unpack_from("<3f", data, off + 84)
        name = struct.unpack_from("<56s", data, off + 96)[0]
        return {
            "featureId": int(feature_id),
            "featureType": int(feature_type),
            "geometryType": int(geometry_type),
            "parentCount": int(parent_count),
            "childCount": int(child_count),
            "elemId": int(elem_id),
            "length": float(length),
            "area": float(area),
            "volume": float(volume),
            "center": [float(x), float(y), float(z)],
            "bboxMin": list(bbox_min),
            "bboxMax": list(bbox_max),
            "name": name.rstrip(b"\x00").decode("utf-8", errors="replace"),
        }

    @staticmethod
    def _parse_geometry_info(data):
        header, err = NativeCommandBridge._parse_geometry_header(data)
        if err:
            return err
        if header["returned"] < 1 or len(data) < 32 + 152:
            return {"ok": False, "error": "几何特征信息结果不完整"}
        feature = NativeCommandBridge._parse_geometry_entry(data, 32)
        return {"ok": True, "result": dict(header, feature=feature)}

    @staticmethod
    def _parse_geometry_features(data):
        header, err = NativeCommandBridge._parse_geometry_header(data)
        if err:
            return err
        features = []
        for i in range(header["returned"]):
            off = 32 + i * 152
            if off + 152 > len(data):
                break
            features.append(NativeCommandBridge._parse_geometry_entry(data, off))
        return {"ok": True, "result": dict(header, features=features)}

    @staticmethod
    def _parse_geometry_import(data):
        """解析 GeometryImportResult（72B）。"""
        if len(data) < 72:
            return {"ok": False, "error": "几何导入结果数据不完整"}
        magic, version, status, flags, data_node_id, vertex_count, edge_count, face_count, \
            solid_count, triangle_count, _reserved, bmin_x, bmin_y, bmin_z, bmax_x, bmax_y, \
            bmax_z = struct.unpack_from("<IIIIQ6I6f", data, 0)
        if magic != 0x4D4F4547:
            return {"ok": False, "error": "几何导入结果 magic 校验失败: 0x%08X" % magic}
        status_msgs = {
            1: "入参非法（空路径/不支持扩展名/文件不存在）",
            2: "几何内核导入失败",
            3: "转换失败（无三角面）",
            4: "数据节点注册失败",
        }
        if status != 0:
            return {"ok": False,
                    "error": "%s（status=%d）" % (status_msgs.get(status, "未知错误"), status)}
        return {
            "ok": True,
            "result": {
                "status": "imported",
                "dataNodeId": int(data_node_id),
                "vertexCount": int(vertex_count),
                "edgeCount": int(edge_count),
                "faceCount": int(face_count),
                "solidCount": int(solid_count),
                "triangleCount": int(triangle_count),
                "bboxMin": [float(bmin_x), float(bmin_y), float(bmin_z)],
                "bboxMax": [float(bmax_x), float(bmax_y), float(bmax_z)],
            },
        }

    @staticmethod
    def _parse_query_result(event_id, data):
        parser = {
            200011: NativeCommandBridge._parse_list_data_tree,
            200012: NativeCommandBridge._parse_mesh_ids,
            200013: NativeCommandBridge._parse_mesh_ids,
            200014: NativeCommandBridge._parse_nodes_xyz,
            200015: NativeCommandBridge._parse_elements_topo,
            200021: NativeCommandBridge._parse_geometry_info,
            200022: NativeCommandBridge._parse_geometry_features,
        }.get(event_id)
        if parser is None:
            return {"ok": False, "error": "未知查询结果 eventId: %d" % event_id}
        return parser(data)

    @staticmethod
    def _serialize(name, args):
        """把工具名+参数序列化为 (eventId, 载荷字节)；返回 None 表示引擎侧尚未实现。"""
        if name == "FitView":
            return 100002, struct.pack("<I", 0)
        if name == "ShowPanel":
            panel = (args.get("panelName") or "").encode("utf-8")
            return 100001, struct.pack("<I", 64) + panel.ljust(64, b"\x00")[:64]
        if name == "HidePanel":
            panel = (args.get("panelName") or "").encode("utf-8")
            return 100004, struct.pack("<I", 64) + panel.ljust(64, b"\x00")[:64]
        if name == "SetCamera":
            mode = 1 if (args.get("mode") or "orbit") == "fps" else 0
            field_mask = 0
            pos = parse_vec3(args.get("position"))
            if pos is None:
                pos = (0.0, 0.0, 0.0)
            else:
                field_mask |= 0x1  # bit0=position
            tgt = parse_vec3(args.get("target"))
            if tgt is None:
                tgt = (0.0, 0.0, 0.0)
            else:
                field_mask |= 0x2  # bit1=target
            fov_raw = args.get("fov")
            fov = float(fov_raw) if is_number(fov_raw) else 0.0
            if is_number(fov_raw):
                field_mask |= 0x4  # bit2=fov
            dur_raw = args.get("duration")
            dur = float(dur_raw) if is_number(dur_raw) else 0.45
            if is_number(dur_raw):
                field_mask |= 0x8  # bit3=duration
            return 300005, struct.pack("<I", 40) + struct.pack("<II3f3fff", mode, field_mask, *pos, *tgt, fov, dur)
        if name == "GetMeshData":
            return 200010, struct.pack("<I", 8) + struct.pack("<Q", int(args.get("dataNodeId") or 0))
        if name == "ListDataTreeNodes":
            return 200011, struct.pack("<I", 0)
        if name in ("GetMeshNodes", "GetMeshElements"):
            node = int(args.get("dataNodeId") or 0)
            mesh_name = (args.get("name") or "").encode("utf-8")
            event_id = 200012 if name == "GetMeshNodes" else 200013
            body = struct.pack("<Q64s", node, mesh_name.ljust(64, b"\x00")[:64])
            return event_id, struct.pack("<I", 72) + body
        if name in ("GetNodesXYZ", "GetElementsTopology"):
            node = int(args.get("dataNodeId") or 0)
            mesh_name = (args.get("name") or "").encode("utf-8")
            ids = parse_ids(args.get("ids"))
            event_id = 200014 if name == "GetNodesXYZ" else 200015
            body = struct.pack("<Q64sII", node, mesh_name.ljust(64, b"\x00")[:64], len(ids), 0)
            body += struct.pack("<%dI" % len(ids), *ids)
            return event_id, struct.pack("<I", 80 + len(ids) * 4) + body
        if name == "MeshImport":
            path = (args.get("fileDir") or "").encode("utf-8")
            return 200002, struct.pack("<I", 128) + path.ljust(128, b"\x00")[:128]
        if name == "MeshDelete":
            return 200003, struct.pack("<I", 8) + struct.pack("<Q", int(args.get("dataNodeId") or 0))
        if name == "Highlight":
            ids = parse_ids(args.get("ids"))[:64]
            body = struct.pack("<I", len(ids)) + struct.pack("<%dQ" % len(ids), *ids)
            return 100003, struct.pack("<I", len(body)) + body
        if name == "LocateElements":
            node = int(args.get("nodeId") or 0)
            ids = parse_ids(args.get("ids"))[:512]
            body = struct.pack("<QI", node, len(ids)) + struct.pack("<%dI" % len(ids), *ids)
            return 200005, struct.pack("<I", len(body)) + body
        if name in ("ElementsVisibility", "NodeVisibility"):
            node = int(args.get("nodeId") or 0)
            mode = int(args.get("mode") or 0)
            ids = parse_ids(args.get("ids"))
            event_id = 200006 if name == "ElementsVisibility" else 200007
            body = struct.pack("<QII", node, mode, len(ids)) + struct.pack("<%dI" % len(ids), *ids)
            return event_id, struct.pack("<I", len(body)) + body
        if name == "SetReferencePlane":
            mask = 0
            enabled = 0
            show_grid = 0
            placement = 1
            scale = 1.2
            colors = [0.0] * 12
            if "enabled" in args:
                enabled = 1 if args.get("enabled") else 0
                mask |= 0x1
            if "showGrid" in args:
                show_grid = 1 if args.get("showGrid") else 0
                mask |= 0x2
            if "placement" in args:
                placement = int(args.get("placement") or 0)
                mask |= 0x4
            if "scale" in args and is_number(args.get("scale")):
                scale = float(args.get("scale"))
                mask |= 0x8
            for index, key in enumerate(("colorX", "colorY", "colorZ")):
                if key in args:
                    color = parse_vec4(args.get(key))
                    if color is not None:
                        base = index * 4
                        colors[base:base + 4] = list(color)
                        mask |= 1 << (4 + index)
            body = struct.pack("<IIIIf12f", mask, enabled, show_grid, placement, scale, *colors)
            return 400001, struct.pack("<I", 68) + body
        if name == "ImportGeometry":
            path = (args.get("filePath") or "").encode("utf-8")
            kernel = (args.get("kernelName") or "").encode("utf-8")
            body = struct.pack("<512s64s", path.ljust(512, b"\x00")[:512],
                               kernel.ljust(64, b"\x00")[:64])
            return 200020, struct.pack("<I", 576) + body
        if name == "GetGeometryInfo":
            node = int(args.get("dataNodeId") or 0)
            model_name = (args.get("name") or "").encode("utf-8")
            # featureType/featureId 缺省 0xFFFFFFFF，elemId 缺省 0；用 in 判断避免把 0 当缺省。
            feature_type = int(args["featureType"]) if "featureType" in args else 0xFFFFFFFF
            feature_id = int(args["featureId"]) if "featureId" in args else 0xFFFFFFFF
            elem_id = int(args["elemId"]) if "elemId" in args else 0
            body = struct.pack("<Q64sIIII", node, model_name.ljust(64, b"\x00")[:64],
                               feature_type, feature_id, elem_id, 0)
            return 200021, struct.pack("<I", 88) + body
        if name == "ListGeometryFeatures":
            node = int(args.get("dataNodeId") or 0)
            model_name = (args.get("name") or "").encode("utf-8")
            feature_type = int(args["featureType"]) if "featureType" in args else 0xFFFFFFFF
            offset = int(args["offset"]) if "offset" in args else 0
            limit = int(args["limit"]) if "limit" in args else 0
            body = struct.pack("<Q64sIIII", node, model_name.ljust(64, b"\x00")[:64],
                               feature_type, offset, limit, 0)
            return 200022, struct.pack("<I", 88) + body
        if name == "SetGeometryRenderMode":
            mode = int(args.get("mode") or 0)
            return 200023, struct.pack("<I", 8) + struct.pack("<II", mode, 0)
        if name == "SetGeometryPickFilter":
            mask = int(args.get("mask") or 0) & 0x0F
            return 200024, struct.pack("<I", 8) + struct.pack("<II", mask, 0)
        return None

    def call(self, name, arguments):
        pid = resolve_engine_pid()
        if pid <= 0:
            return {"ok": False,
                    "error": "EnginePlatform 未运行（注册表无有效 PID，且未发现 EnginePlatform.exe 进程）"}
        if self._pid != pid:
            if not self._connect(pid):
                return {"ok": False,
                        "error": "无法打开 EnginePlatform 命令队列（共享内存 Command Memory-%d）" % pid}
        serialized = self._serialize(name, arguments)
        if serialized is None:
            return {"ok": False,
                    "error": "命令 %s 尚未在引擎侧实现（当前仅支持已注册的 CommandManager 命令）" % name}
        event_id, payload = serialized
        if not self._push(event_id, payload):
            return {"ok": False, "error": "EnginePlatform 命令队列已满，请稍后重试"}
        query_commands = {"GetMeshData", "ListDataTreeNodes", "GetMeshNodes",
                          "GetMeshElements", "GetNodesXYZ", "GetElementsTopology",
                          "ImportGeometry", "GetGeometryInfo", "ListGeometryFeatures"}
        if name in query_commands:
            data, err = self._read_result(pid, event_id)
            if err:
                return {"ok": False, "error": err}
            if name == "GetMeshData":
                return self._parse_mesh_result(data)
            if name == "ImportGeometry":
                return self._parse_geometry_import(data)
            return self._parse_query_result(event_id, data)
        return {"ok": True,
                "result": {"status": "dispatched", "eventId": event_id, "command": name}}


# ---------------------------------------------------------------------------
# 本地配置（DeepSeek API Key 等，无需环境变量）
# ---------------------------------------------------------------------------

def default_config_path():
    """默认配置文件路径：%APPDATA%/EgLab/agent_config.json（非 Windows 回退到 ~/.eglab）。"""
    appdata = os.environ.get("APPDATA")
    if appdata:
        return os.path.join(appdata, "EgLab", "agent_config.json")
    return os.path.join(os.path.expanduser("~"), ".eglab", "agent_config.json")


def load_config(path):
    try:
        with open(path, "r", encoding="utf-8") as f:
            data = json.load(f)
        return data if isinstance(data, dict) else {}
    except (OSError, json.JSONDecodeError):
        return {}


def save_config(path, data):
    dirpath = os.path.dirname(path)
    if dirpath:
        os.makedirs(dirpath, exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)


def mask_key(key):
    key = key or ""
    if not key:
        return ""
    if len(key) <= 8:
        return "*" * len(key)
    return key[:6] + "****" + key[-4:]


# ---------------------------------------------------------------------------
# DeepSeek 客户端（流式）
# ---------------------------------------------------------------------------

def stream_chat(api_key, messages, tools, model, on_delta, base_url=DEEPSEEK_API_URL):
    """流式调用 DeepSeek。on_delta(text) 回调文本增量；返回 (content, tool_calls)。

    tool_calls 形如 [{"id","name","arguments(json str)"}, ...]。
    """
    payload = {
        "model": model,
        "messages": messages,
        "tools": tools,
        "tool_choice": "auto",
        "temperature": 0.0,
        "stream": True,
    }
    request = urllib.request.Request(
        base_url,
        data=json.dumps(payload).encode("utf-8"),
        headers={
            "Content-Type": "application/json",
            "Authorization": "Bearer " + api_key,
            "Accept": "text/event-stream",
        },
        method="POST",
    )
    content_parts = []
    slots = {}  # index -> {id, name, arguments}

    try:
        with urllib.request.urlopen(request, timeout=120) as response:
            for raw_line in response:
                try:
                    line = raw_line.decode("utf-8").strip()
                except UnicodeDecodeError:
                    continue
                if not line.startswith("data:"):
                    continue
                data = line[5:].strip()
                if data == "[DONE]":
                    break
                try:
                    chunk = json.loads(data)
                except json.JSONDecodeError:
                    continue
                choices = chunk.get("choices") or []
                if not choices:
                    continue
                delta = choices[0].get("delta") or {}
                text = delta.get("content")
                if text:
                    content_parts.append(text)
                    on_delta(text)
                for tc in delta.get("tool_calls") or []:
                    idx = tc.get("index", 0)
                    slot = slots.setdefault(
                        idx, {"id": None, "name": "", "arguments": ""})
                    if tc.get("id"):
                        slot["id"] = tc["id"]
                    fn = tc.get("function") or {}
                    if fn.get("name"):
                        slot["name"] = fn["name"]
                    if fn.get("arguments"):
                        slot["arguments"] += fn["arguments"]
    except urllib.error.HTTPError as exc:
        detail = exc.read().decode("utf-8", errors="replace")
        raise RuntimeError("DeepSeek API 返回 HTTP %s: %s" % (exc.code, detail)) from exc

    content = "".join(content_parts)
    tool_calls = [slots[i] for i in sorted(slots)]
    return content, tool_calls


# ---------------------------------------------------------------------------
# CommandBridge：把工具调用转发给引擎（stub 演示后端 / 真实 MCP 后端）
# ---------------------------------------------------------------------------

class StubBridge:
    """无引擎时的演示后端，模拟 MCP Server 行为，用于端到端联调 UI 与 Agent 循环。"""

    def __init__(self):
        self._next_node_id = 1000

    def list_tools(self):
        return []

    def call(self, name, arguments):
        time.sleep(0.25)  # 模拟引擎执行耗时，便于 UI 展示进度
        if name == "CreateMesh":
            self._next_node_id += 1
            return {
                "ok": True,
                "result": {
                    "status": "created",
                    "dataNodeId": self._next_node_id,
                    "geometryType": arguments.get("geometryType", "box"),
                    "name": arguments.get("name") or ("Mesh_%d" % self._next_node_id),
                    "size": arguments.get("size", 1.0),
                    "subdivisions": arguments.get("subdivisions", 16),
                },
            }
        if name == "SetCamera":
            pos = parse_vec3(arguments.get("position"))
            tgt = parse_vec3(arguments.get("target"))
            return {
                "ok": True,
                "result": {
                    "status": "applied",
                    "mode": arguments.get("mode", "fit"),
                    "position": list(pos) if pos is not None else arguments.get("position"),
                    "target": list(tgt) if tgt is not None else arguments.get("target"),
                    "fov": arguments.get("fov"),
                    "duration": arguments.get("duration"),
                },
            }
        if name == "GetLog":
            count = min(arguments.get("count") or 50, 500)
            level = arguments.get("level") or "all"
            keyword = arguments.get("filter") or ""
            logs = self._fake_logs(count, level, keyword)
            return {"ok": True, "result": {"count": len(logs), "logs": logs}}
        if name == "MeshImport":
            self._next_node_id += 1
            return {
                "ok": True,
                "result": {
                    "status": "imported",
                    "dataNodeId": self._next_node_id,
                    "fileDir": arguments.get("fileDir", ""),
                },
            }
        if name == "MeshDelete":
            return {
                "ok": True,
                "result": {"status": "deleted", "dataNodeId": arguments.get("dataNodeId", 0)},
            }
        if name == "GetMeshData":
            node_id = int(arguments.get("dataNodeId") or 0)
            return {
                "ok": True,
                "result": {
                    "status": "ok",
                    "meshCount": 1,
                    "truncated": False,
                    "totalNodeCount": 1564,
                    "totalElemCount": 3200,
                    "totalBBox": {"min": [-1.0, -1.0, -1.0], "max": [1.0, 1.0, 1.0]},
                    "meshes": [
                        {
                            "dataNodeId": node_id,
                            "name": "DemoMesh_%d" % node_id,
                            "nodeCount": 1564,
                            "elemCount": 3200,
                            "bboxMin": [-1.0, -1.0, -1.0],
                            "bboxMax": [1.0, 1.0, 1.0],
                            "elemTypeCount": 2,
                            "elemTypes": [
                                {"id": 4, "count": 2800},
                                {"id": 5, "count": 400},
                            ],
                        }
                    ],
                },
            }
        if name == "ListDataTreeNodes":
            node_id = int(arguments.get("dataNodeId") or 1000)
            return {
                "ok": True,
                "result": {
                    "status": "ok",
                    "total": 1,
                    "offset": 0,
                    "returned": 1,
                    "nodes": [
                        {"dataNodeId": node_id, "name": "DemoMesh_%d" % node_id,
                         "kind": 1, "nodeCount": 1564, "elemCount": 3200},
                    ],
                },
            }
        if name == "GetMeshNodes":
            return {
                "ok": True,
                "result": {"status": "ok", "total": 1564, "offset": 0, "returned": 8,
                           "ids": [1, 2, 3, 4, 5, 6, 7, 8]},
            }
        if name == "GetMeshElements":
            return {
                "ok": True,
                "result": {"status": "ok", "total": 3200, "offset": 0, "returned": 4,
                           "ids": [1, 2, 3, 4]},
            }
        if name == "GetNodesXYZ":
            ids = parse_ids(arguments.get("ids"))[:3]
            return {
                "ok": True,
                "result": {"status": "ok", "total": len(ids), "offset": 0,
                           "returned": len(ids),
                           "nodes": [{"id": i, "xyz": [round(4112.0 + i * 0.001, 6),
                                                        round(-468.0, 6),
                                                        round(322.0 + i * 0.1, 6)]}
                                     for i in ids]},
            }
        if name == "GetElementsTopology":
            ids = parse_ids(arguments.get("ids"))[:3]
            return {
                "ok": True,
                "result": {"status": "ok", "total": len(ids), "offset": 0,
                           "returned": len(ids),
                           "elements": [{"id": i, "type": 0x204, "nodeCount": 4,
                                         "nodeIds": [i, i + 1, i + 2, i + 3]}
                                        for i in ids]},
            }
        if name == "ShowPanel":
            return {
                "ok": True,
                "result": {"status": "queued", "panelName": arguments.get("panelName", "")},
            }
        if name == "HidePanel":
            return {
                "ok": True,
                "result": {"status": "hidden", "panelName": arguments.get("panelName", "")},
            }
        if name == "FitView":
            return {"ok": True, "result": {"status": "applied"}}
        if name == "Highlight":
            return {
                "ok": True,
                "result": {"status": "highlighted", "ids": arguments.get("ids", "")},
            }
        if name == "LocateElements":
            return {
                "ok": True,
                "result": {
                    "status": "located",
                    "nodeId": arguments.get("nodeId", 0),
                    "ids": arguments.get("ids", ""),
                },
            }
        if name in ("ElementsVisibility", "NodeVisibility"):
            return {
                "ok": True,
                "result": {
                    "status": "applied",
                    "nodeId": arguments.get("nodeId", 0),
                    "mode": arguments.get("mode", 0),
                    "ids": arguments.get("ids", ""),
                },
            }
        return {"ok": False, "error": "未知工具: %s" % name}

    @staticmethod
    def _fake_logs(count, level, keyword):
        levels = ["info", "info", "debug", "warning", "error"]
        messages = [
            "Command ImportMeshCmd finished, elapsed_ms=12.4",
            "RenderWork::commitPendingRenderUpdates ok",
            "Mesh bounds recomputed for node 1001",
            "Shader compiled successfully",
            "Frame dropped: draw call budget exceeded",
        ]
        out = []
        for i in range(count):
            lv = levels[i % len(levels)]
            if level != "all" and lv != level:
                continue
            msg = messages[i % len(messages)]
            if keyword and keyword not in msg:
                continue
            out.append({"time": time.strftime("%H:%M:%S"), "level": lv, "message": msg})
        return out


class McpBridge:
    """通过 JSON-RPC 2.0 over NDJSON 连接 C++ 内嵌 MCP Server（Windows Named Pipe）。

    协议与蓝图（docs/AI_IPC_Blueprint.md §3）一致：一行一条 JSON、`\\n` 分帧，
    方法 tools/list、tools/call。Python 侧用 ctypes 直接打开命名管道，零第三方依赖。
    注：变更类工具的异步结果通知（notifications/toolResult）属 P1，待 C++ 服务端
    落地后再做结果关联，本桥当前为同步 request/response。
    """

    GENERIC_READ = 0x80000000
    GENERIC_WRITE = 0x40000000
    OPEN_EXISTING = 3
    INVALID_HANDLE_VALUE = ctypes.c_void_p(-1).value

    def __init__(self, pipe_name):
        self.pipe_name = pipe_name
        self._id = 0
        k = ctypes.windll.kernel32
        k.CreateFileW.restype = ctypes.c_void_p
        k.CreateFileW.argtypes = [ctypes.c_wchar_p, ctypes.c_uint32, ctypes.c_uint32,
                                  ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint32,
                                  ctypes.c_void_p]
        k.WriteFile.restype = ctypes.c_int
        k.WriteFile.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_uint32,
                                ctypes.POINTER(ctypes.c_uint32), ctypes.c_void_p]
        k.ReadFile.restype = ctypes.c_int
        k.ReadFile.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_uint32,
                               ctypes.POINTER(ctypes.c_uint32), ctypes.c_void_p]
        k.CloseHandle.argtypes = [ctypes.c_void_p]
        self._kernel32 = k

    def _next_id(self):
        self._id += 1
        return self._id

    def _request_line(self, payload_json):
        path = "\\\\.\\pipe\\" + self.pipe_name
        handle = self._kernel32.CreateFileW(path, self.GENERIC_READ | self.GENERIC_WRITE,
                                            0, None, self.OPEN_EXISTING, 0, None)
        if handle == self.INVALID_HANDLE_VALUE:
            raise RuntimeError(
                "无法打开命名管道 %s（EnginePlatform 内嵌 MCP Server 可能未启动）" % path)
        try:
            data = payload_json.encode("utf-8") + b"\n"
            written = ctypes.c_uint32(0)
            if not self._kernel32.WriteFile(handle, data, len(data),
                                            ctypes.byref(written), None):
                raise RuntimeError("写入命名管道失败: %s" % path)
            buf = bytearray()
            chunk = ctypes.create_string_buffer(4096)
            while not buf.endswith(b"\n"):
                read = ctypes.c_uint32(0)
                if not self._kernel32.ReadFile(handle, chunk, 4096,
                                               ctypes.byref(read), None) or read.value == 0:
                    break
                buf += chunk.raw[:read.value]
        finally:
            self._kernel32.CloseHandle(handle)
        return bytes(buf)

    def _rpc(self, method, params=None):
        payload = {"jsonrpc": "2.0", "id": self._next_id(), "method": method,
                   "params": params or {}}
        raw = self._request_line(json.dumps(payload, ensure_ascii=False))
        if not raw:
            raise RuntimeError("MCP Server 无响应")
        response = json.loads(raw.decode("utf-8"))
        if "error" in response:
            return {"ok": False, "error": response["error"]}
        return {"ok": True, "result": response.get("result")}

    def list_tools(self):
        resp = self._rpc("tools/list")
        if not resp["ok"]:
            return []
        return resp["result"].get("tools", [])

    def call(self, name, arguments):
        return self._rpc("tools/call", {"name": name, "arguments": arguments})


# ---------------------------------------------------------------------------
# 会话与事件（SSE）管理
# ---------------------------------------------------------------------------

class EventBroker:
    """按会话分发事件，支持 SSE 断线重连时按 last_event_id 重放。"""

    def __init__(self):
        self._lock = threading.Lock()
        self._subs = {}   # session_id -> list[queue]
        self._logs = {}   # session_id -> {seq: event}
        self._seq = {}    # session_id -> next seq

    def emit(self, session_id, event_type, data):
        with self._lock:
            seq = self._seq.get(session_id, 0)
            self._seq[session_id] = seq + 1
            event = {"id": seq, "event": event_type, "data": data}
            self._logs.setdefault(session_id, {})[seq] = event
            for q in self._subs.get(session_id, []):
                q.put(event)

    def subscribe(self, session_id):
        q = queue.Queue()
        with self._lock:
            self._subs.setdefault(session_id, []).append(q)
        return q

    def unsubscribe(self, session_id, q):
        with self._lock:
            subs = self._subs.get(session_id)
            if subs and q in subs:
                subs.remove(q)

    def replay(self, session_id, last_event_id):
        with self._lock:
            next_seq = self._seq.get(session_id, 0)
            events = [
                self._logs.get(session_id, {}).get(i)
                for i in range(last_event_id + 1, next_seq)
            ]
            return [e for e in events if e is not None]


class SessionStore:
    def __init__(self):
        self._lock = threading.Lock()
        self._sessions = {}

    def create(self):
        session_id = "%d" % int(time.time() * 1000)
        with self._lock:
            self._sessions[session_id] = {
                "id": session_id,
                "title": "新会话",
                "created": time.time(),
                "messages": [{"role": "system", "content": SYSTEM_PROMPT}],
                "running": False,
            }
        return session_id

    def get(self, session_id):
        with self._lock:
            return self._sessions.get(session_id)

    def list(self):
        with self._lock:
            return [
                {
                    "id": s["id"],
                    "title": s["title"],
                    "created": s["created"],
                    "message_count": len([m for m in s["messages"] if m["role"] != "system"]),
                }
                for s in self._sessions.values()
            ]

    def append_message(self, session_id, message, set_title=None):
        with self._lock:
            s = self._sessions.get(session_id)
            if not s:
                return False
            s["messages"].append(message)
            if set_title and s["title"] == "新会话":
                s["title"] = set_title[:20]
            return True

    def messages(self, session_id):
        with self._lock:
            s = self._sessions.get(session_id)
            return list(s["messages"]) if s else None

    def set_running(self, session_id, running):
        with self._lock:
            s = self._sessions.get(session_id)
            if s:
                s["running"] = running


# ---------------------------------------------------------------------------
# Agent 循环
# ---------------------------------------------------------------------------

class Agent:
    def __init__(self, api_key, model, bridge, broker, sessions, tools,
                 base_url=DEEPSEEK_API_URL):
        self.api_key = api_key
        self.model = model
        self.base_url = base_url
        self.bridge = bridge
        self.broker = broker
        self.sessions = sessions
        self.tools = tools

    def run(self, session_id, user_text):
        session = self.sessions.get(session_id)
        if session is None:
            return
        session["running"] = True
        try:
            if not self.api_key:
                self.broker.emit(session_id, "error",
                                 {"message": "尚未设置 DeepSeek API Key，请点击左下角“设置”填写后重试"})
                return
            self.sessions.append_message(session_id, {"role": "user", "content": user_text},
                                         set_title=user_text)
            messages = self.sessions.messages(session_id)
            self.broker.emit(session_id, "status", {"phase": "thinking", "message": "正在解析意图..."})

            for step in range(MAX_AGENT_STEPS):
                self.broker.emit(session_id, "status",
                                 {"phase": "llm", "message": "调用 DeepSeek (%d/%d)" % (step + 1, MAX_AGENT_STEPS)})
                content, tool_calls = stream_chat(
                    self.api_key, messages, self.tools, self.model,
                    on_delta=lambda t: self.broker.emit(session_id, "text_delta", {"delta": t}),
                    base_url=self.base_url)

                assistant_msg = {"role": "assistant", "content": content or None}
                if tool_calls:
                    assistant_msg["tool_calls"] = [
                        {"id": c["id"] or ("call_%d" % i), "type": "function",
                         "function": {"name": c["name"], "arguments": c["arguments"]}}
                        for i, c in enumerate(tool_calls)
                    ]
                self.sessions.append_message(session_id, assistant_msg)

                if not tool_calls:
                    self.broker.emit(session_id, "status", {"phase": "done", "message": "完成"})
                    self.broker.emit(session_id, "done", {"content": content})
                    return

                for i, call in enumerate(tool_calls):
                    try:
                        args = json.loads(call["arguments"] or "{}")
                    except json.JSONDecodeError:
                        args = {}
                    self.broker.emit(session_id, "tool_call",
                                     {"step": i + 1, "total": len(tool_calls),
                                      "name": call["name"], "arguments": args})
                    self.broker.emit(session_id, "status",
                                     {"phase": "executing", "message": "执行 %s..." % call["name"]})
                    result = self.bridge.call(call["name"], args)
                    self.broker.emit(session_id, "tool_result",
                                     {"name": call["name"], "ok": result.get("ok", False),
                                      "result": result.get("result"), "error": result.get("error")})
                    tool_msg = {
                        "role": "tool",
                        "tool_call_id": call["id"] or ("call_%d" % i),
                        "content": json.dumps(result, ensure_ascii=False),
                    }
                    self.sessions.append_message(session_id, tool_msg)

                # 继续循环，让模型基于工具结果给出下一步或最终回答
                messages = self.sessions.messages(session_id)
                self.broker.emit(session_id, "status", {"phase": "thinking", "message": "综合工具结果..."})

            self.broker.emit(session_id, "error",
                             {"message": "工具调用步数超过上限 (%d)" % MAX_AGENT_STEPS})
        except Exception as exc:  # noqa: BLE001
            self.broker.emit(session_id, "error", {"message": str(exc)})
        finally:
            self.sessions.set_running(session_id, False)


# ---------------------------------------------------------------------------
# HTTP + SSE 服务
# ---------------------------------------------------------------------------

class Handler(BaseHTTPRequestHandler):
    server_version = "EgLabAgent/1.0"

    # 通过类属性注入依赖
    sessions = None
    broker = None
    agent = None
    tools = None
    config_path = None
    config = {}
    config_lock = None

    def log_message(self, fmt, *args):  # 安静模式
        pass

    def _send_json(self, obj, status=200):
        body = json.dumps(obj, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(body)

    def _read_json_body(self):
        length = int(self.headers.get("Content-Length") or 0)
        if length <= 0:
            return {}
        raw = self.rfile.read(length)
        for encoding in ("utf-8", "gbk", "utf-16"):
            try:
                return json.loads(raw.decode(encoding))
            except (UnicodeDecodeError, json.JSONDecodeError):
                continue
        return {}

    def do_OPTIONS(self):
        self.send_response(204)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()

    def do_GET(self):
        path = self.path.split("?", 1)[0]
        if path in ("/", "/index.html"):
            self._serve_ui()
        elif path == "/api/health":
            self._send_json({"ok": True, "service": "EgLab-Agent"})
        elif path == "/api/engine":
            pid = read_engine_pid()
            self._send_json({"pid": pid, "running": is_pid_running(pid)})
        elif path == "/api/sessions":
            self._send_json({"sessions": self.sessions.list()})
        elif path == "/api/history":
            query = {}
            if "?" in self.path:
                for pair in self.path.split("?", 1)[1].split("&"):
                    if "=" in pair:
                        key, value = pair.split("=", 1)
                        query[key] = urllib.parse.unquote(value)
            session_id = query.get("session_id", "")
            messages = self.sessions.messages(session_id)
            if messages is None:
                self._send_json({"error": "会话不存在"}, status=404)
            else:
                visible = [m for m in messages if m.get("role") != "system"]
                self._send_json({"session_id": session_id, "messages": visible})
        elif path == "/api/tools":
            self._send_json({"tools": self.tools})
        elif path == "/api/config":
            self._send_json(self._get_config())
        elif path == "/api/stream":
            self._serve_stream()
        else:
            self._send_json({"error": "not found"}, status=404)

    def do_POST(self):
        path = self.path.split("?", 1)[0]
        body = self._read_json_body()
        if path == "/api/sessions":
            session_id = self.sessions.create()
            self._send_json({"session_id": session_id})
        elif path == "/api/chat":
            session_id = body.get("session_id")
            message = (body.get("message") or "").strip()
            if not session_id or not self.sessions.get(session_id):
                self._send_json({"error": "会话不存在"}, status=404)
            elif not message:
                self._send_json({"error": "消息为空"}, status=400)
            elif self.sessions.get(session_id)["running"]:
                self._send_json({"error": "该会话正在处理上一条消息"}, status=409)
            else:
                threading.Thread(target=self.agent.run, args=(session_id, message),
                                 daemon=True).start()
                self._send_json({"session_id": session_id, "started": True})
        elif path == "/api/config":
            self._update_config(body)
        else:
            self._send_json({"error": "not found"}, status=404)

    def _get_config(self):
        with self.config_lock:
            key = self.config.get("deepseek_api_key", "")
            return {
                "has_key": bool(key),
                "masked_key": mask_key(key),
                "model": self.config.get("deepseek_model") or DEFAULT_MODEL,
                "base_url": self.config.get("deepseek_base_url") or DEEPSEEK_API_URL,
            }

    def _update_config(self, body):
        key = (body.get("deepseek_api_key") or "").strip()
        model = (body.get("model") or "").strip()
        base_url = (body.get("base_url") or "").strip()
        with self.config_lock:
            if key:
                self.config["deepseek_api_key"] = key
                self.agent.api_key = key
            if model:
                self.config["deepseek_model"] = model
                self.agent.model = model
            if base_url:
                self.config["deepseek_base_url"] = base_url
                self.agent.base_url = base_url
            try:
                save_config(self.config_path, self.config)
            except OSError as exc:
                self._send_json({"error": "保存配置失败: %s" % exc}, status=500)
                return
        self._send_json(self._get_config())

    def _serve_ui(self):
        if not os.path.exists(UI_INDEX_PATH):
            self._send_json({"error": "ui/index.html 不存在"}, status=404)
            return
        with open(UI_INDEX_PATH, "rb") as f:
            body = f.read()
        self.send_response(200)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def _serve_stream(self):
        query = {}
        if "?" in self.path:
            for pair in self.path.split("?", 1)[1].split("&"):
                if "=" in pair:
                    key, value = pair.split("=", 1)
                    query[key] = urllib.parse.unquote(value)
        session_id = query.get("session_id", "")
        last_event_id = int(query.get("last_event_id", -1))
        if not session_id or not self.sessions.get(session_id):
            self._send_json({"error": "会话不存在"}, status=404)
            return

        self.send_response(200)
        self.send_header("Content-Type", "text/event-stream; charset=utf-8")
        self.send_header("Cache-Control", "no-cache")
        self.send_header("Connection", "keep-alive")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()

        q = self.broker.subscribe(session_id)
        try:
            for event in self.broker.replay(session_id, last_event_id):
                self._write_sse(event)
            self.wfile.flush()
            while True:
                try:
                    event = q.get(timeout=15)
                except queue.Empty:
                    self.wfile.write(b": ping\n\n")
                    self.wfile.flush()
                    continue
                self._write_sse(event)
        except (BrokenPipeError, ConnectionResetError):
            pass
        finally:
            self.broker.unsubscribe(session_id, q)

    def _write_sse(self, event):
        payload = json.dumps(event["data"], ensure_ascii=False)
        lines = "id: %d\nevent: %s\ndata: %s\n\n" % (event["id"], event["event"], payload)
        self.wfile.write(lines.encode("utf-8"))
        self.wfile.flush()


def load_mcp_tools(path=MCP_TOOLS_PATH):
    with open(path, "r", encoding="utf-8") as f:
        doc = json.load(f)
    return [
        {"type": "function",
         "function": {"name": t["name"], "description": t["description"],
                      "parameters": t["parameters"]}}
        for t in doc["tools"]
    ]


def load_command_tools(path=COMMAND_SCHEMAS_PATH):
    """加载 command_schemas.json 中的 Command，转换为 function-calling 工具。"""
    with open(path, "r", encoding="utf-8") as f:
        doc = json.load(f)
    return [
        {"type": "function",
         "function": {"name": c["name"], "description": c["description"],
                      "parameters": c["parameters"]}}
        for c in doc["commands"]
    ]


def load_all_tools(mcp_path=MCP_TOOLS_PATH, cmd_path=COMMAND_SCHEMAS_PATH):
    """合并 MCP 工具层与 Command 层，得到 AI 可调用的完整工具集。

    以 mcp_tools.json 为对外工具层优先，再补入 command_schemas.json 中
    未被 MCP 工具覆盖的 Command（如 MeshImport/MeshDelete/Highlight 等），
    使「导入网格、高亮、定位、可见性」等命令同样对 AI 生效。
    """
    tools = load_mcp_tools(mcp_path)
    names = {t["function"]["name"] for t in tools}
    for cmd in load_command_tools(cmd_path):
        if cmd["function"]["name"] not in names:
            tools.append(cmd)
            names.add(cmd["function"]["name"])
    return tools


def main():
    _ensure_utf8_console()
    parser = argparse.ArgumentParser(description="EgLab-Agent 独立进程")
    parser.add_argument("--api-key", default=None,
                        help="DeepSeek API Key（优先于配置文件；也可用环境变量 DEEPSEEK_API_KEY）")
    parser.add_argument("--model", default=None, help="DeepSeek 模型名（默认 deepseek-chat）")
    parser.add_argument("--base-url", default=None, help="DeepSeek API 地址")
    parser.add_argument("--config", default=None,
                        help="配置文件路径（默认 %%APPDATA%%/EgLab/agent_config.json）")
    parser.add_argument("--host", default=DEFAULT_HOST)
    parser.add_argument("--port", type=int, default=DEFAULT_HTTP_PORT)
    parser.add_argument("--bridge", choices=["native", "stub", "mcp"], default="native",
                        help="命令桥：native=直连运行中 EnginePlatform（真实命令），stub=演示，mcp=内嵌 MCP Server（P1）")
    parser.add_argument("--mcp-pipe", default="eglab_mcp",
                        help="C++ 内嵌 MCP Server 的命名管道名（Windows Named Pipe）")
    args = parser.parse_args()

    config_path = args.config or default_config_path()
    config = load_config(config_path)
    api_key = args.api_key or os.environ.get("DEEPSEEK_API_KEY") \
        or config.get("deepseek_api_key", "")
    model = args.model or config.get("deepseek_model") or DEFAULT_MODEL
    base_url = args.base_url or config.get("deepseek_base_url") or DEEPSEEK_API_URL

    if not api_key:
        print("[warn] 尚未设置 DeepSeek API Key：可在对话框中点击“设置”填写，"
              "或通过 --api-key / DEEPSEEK_API_KEY 提供。")
    print("[info] 配置文件: %s" % config_path)

    if args.bridge == "mcp":
        bridge = McpBridge(args.mcp_pipe)
        print("[info] 使用 MCP 后端（Named Pipe）: \\\\.\\pipe\\%s" % args.mcp_pipe)
    elif args.bridge == "native":
        bridge = NativeCommandBridge()
        print("[info] 使用 native 后端：直连运行中 EnginePlatform 的 CommandManager（真实命令执行）")
    else:
        bridge = StubBridge()
        print("[info] 使用 stub 演示后端（不触达真实引擎）。")

    tools = load_all_tools()
    sessions = SessionStore()
    broker = EventBroker()
    agent = Agent(api_key, model, bridge, broker, sessions, tools, base_url=base_url)

    Handler.sessions = sessions
    Handler.broker = broker
    Handler.agent = agent
    Handler.tools = tools
    Handler.config_path = config_path
    Handler.config = config
    Handler.config_lock = threading.Lock()

    httpd = ThreadingHTTPServer((args.host, args.port), Handler)
    url = "http://%s:%d" % (args.host, args.port)
    print("[ready] EgLab-Agent 已启动: %s" % url)
    print("[ready] 已注册工具: %s" % ", ".join(
        t["function"]["name"] for t in tools))
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        httpd.server_close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
