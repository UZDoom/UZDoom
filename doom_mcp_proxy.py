import sys
import json
import threading
import time
import requests
import base64
import logging

# Configuration
DOOM_HOST = "http://localhost:9090"
SSE_URL = f"{DOOM_HOST}/sse"
MCP_URL = f"{DOOM_HOST}/mcp"
TOOL_URL = f"{DOOM_HOST}/tools/call"
SCREENSHOT_URL = f"{DOOM_HOST}/screenshot/latest.png"

# Global state
latest_snapshot = {}
latest_screenshot_meta = {}
lock = threading.Lock()

logging.basicConfig(stream=sys.stderr, level=logging.INFO, format="[Proxy] %(message)s")


def sse_listener():
    global latest_snapshot, latest_screenshot_meta
    while True:
        try:
            logging.info(f"Connecting to SSE: {SSE_URL}")
            response = requests.get(SSE_URL, stream=True)
            if response.status_code != 200:
                logging.error(f"SSE connection failed: {response.status_code}")
                time.sleep(2)
                continue

            for line in response.iter_lines():
                if line:
                    decoded_line = line.decode("utf-8")
                    if decoded_line.startswith("event: state"):
                        # The next line should be data:
                        continue
                    if decoded_line.startswith("event: screenshot"):
                        continue
                    if decoded_line.startswith("data: "):
                        data_str = decoded_line[6:]
                        try:
                            data = json.loads(data_str)
                            # We don't know which event it belongs to strictly without state tracking
                            # But based on schema, snapshots have 'player' and 'level'
                            # Screenshot meta has 'uri'
                            with lock:
                                if "player" in data:
                                    latest_snapshot = data
                                elif "uri" in data:
                                    latest_screenshot_meta = data
                        except json.JSONDecodeError:
                            pass
        except Exception as e:
            logging.error(f"SSE error: {e}")
            time.sleep(2)


def handle_get_state():
    with lock:
        return latest_snapshot


def handle_capture_screenshot():
    # Trigger screenshot
    payload = {
        "jsonrpc": "2.0",
        "method": "tools/call",
        "params": {"name": "capture_screenshot"},
        "id": 1,
    }
    try:
        res = requests.post(TOOL_URL, json=payload)
        if res.status_code != 200:
            return {"error": f"Failed to request screenshot: {res.text}"}

        # Wait for screenshot to update (simple polling for now, or just return latest if close enough)
        # ideally we wait for the version to increment, but we don't track version here easily yet
        # For simplicity, we sleep briefly then fetch latest
        time.sleep(0.2)

        # Fetch the actual image
        img_res = requests.get(SCREENSHOT_URL)
        if img_res.status_code == 200:
            b64_img = base64.b64encode(img_res.content).decode("utf-8")
            return {
                "type": "image",
                "data": b64_img,
                "mimeType": "image/png",
                "metadata": latest_screenshot_meta,
            }
        else:
            return {"error": "Failed to download screenshot"}

    except Exception as e:
        return {"error": str(e)}


def main():
    # Start SSE thread
    t = threading.Thread(target=sse_listener, daemon=True)
    t.start()

    # JSON-RPC over Stdio loop
    while True:
        try:
            line = sys.stdin.readline()
            if not line:
                break

            request = json.loads(line)
            response = {"jsonrpc": "2.0", "id": request.get("id")}

            method = request.get("method")

            if method == "initialize":
                response["result"] = {
                    "protocolVersion": "2024-11-05",
                    "capabilities": {"tools": {}},
                    "serverInfo": {"name": "doom-mcp-proxy", "version": "0.1.0"},
                }

            elif method == "notifications/initialized":
                # No response needed for notifications
                continue

            elif method == "tools/list":
                response["result"] = {
                    "tools": [
                        {
                            "name": "get_game_state",
                            "description": "Get the current state of the Doom game (player health, ammo, nearby enemies, etc.)",
                            "inputSchema": {"type": "object", "properties": {}},
                        },
                        {
                            "name": "capture_screenshot",
                            "description": "Capture a screenshot of the current game view",
                            "inputSchema": {"type": "object", "properties": {}},
                        },
                    ]
                }

            elif method == "tools/call":
                params = request.get("params", {})
                name = params.get("name")

                if name == "get_game_state":
                    response["result"] = {
                        "content": [
                            {
                                "type": "text",
                                "text": json.dumps(handle_get_state(), indent=2),
                            }
                        ]
                    }
                elif name == "capture_screenshot":
                    result = handle_capture_screenshot()
                    if "error" in result:
                        response["result"] = {
                            "content": [
                                {"type": "text", "text": f"Error: {result['error']}"}
                            ],
                            "isError": True,
                        }
                    else:
                        # MCP image format
                        response["result"] = {
                            "content": [
                                {
                                    "type": "image",
                                    "data": result["data"],
                                    "mimeType": "image/png",
                                }
                            ]
                        }
                else:
                    response["error"] = {"code": -32601, "message": "Method not found"}

            else:
                # Ignore other methods or return error
                pass
                # response["error"] = {"code": -32601, "message": "Method not found"}

            if "id" in request:
                print(json.dumps(response))
                sys.stdout.flush()

        except json.JSONDecodeError:
            continue
        except Exception as e:
            logging.error(f"Error: {e}")


if __name__ == "__main__":
    main()
