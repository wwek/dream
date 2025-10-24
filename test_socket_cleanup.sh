#!/bin/bash
# 测试 Socket 文件清理功能

set -e

SOCKET_PATH="/tmp/test_cleanup_drm.sock"
TEST_INPUT="test_input.iq"

echo "========================================="
echo "DRM Status Socket 清理功能测试"
echo "========================================="
echo

# 1. 创建测试输入文件（如果不存在）
if [ ! -f "$TEST_INPUT" ]; then
    echo "Creating test input file..."
    dd if=/dev/zero of=$TEST_INPUT bs=1M count=1 2>/dev/null
fi

# 2. 启动 Dream
echo "Step 1: Starting Dream..."
./dream --mode receive \
    --input-file "$TEST_INPUT" \
    --status-socket "$SOCKET_PATH" &
DREAM_PID=$!
echo "  Dream PID: $DREAM_PID"

# 等待 socket 文件创建
sleep 2

# 3. 验证 socket 文件存在
echo
echo "Step 2: Verifying socket file..."
if [ -S "$SOCKET_PATH" ]; then
    echo "  ✓ Socket file exists: $SOCKET_PATH"
    ls -la "$SOCKET_PATH"
else
    echo "  ✗ Socket file not found!"
    kill $DREAM_PID 2>/dev/null || true
    exit 1
fi

# 4. 测试连接
echo
echo "Step 3: Testing socket connection..."
timeout 2 socat - UNIX-CONNECT:$SOCKET_PATH | head -1 > /dev/null 2>&1
if [ $? -eq 0 ] || [ $? -eq 124 ]; then
    echo "  ✓ Socket connection works"
else
    echo "  ✗ Socket connection failed"
fi

# 5. 正常停止 Dream (Ctrl+C)
echo
echo "Step 4: Stopping Dream with SIGINT (Ctrl+C)..."
kill -INT $DREAM_PID
sleep 2

# 6. 验证 socket 文件是否被删除
echo
echo "Step 5: Verifying socket file cleanup..."
if [ -S "$SOCKET_PATH" ]; then
    echo "  ✗ FAILED: Socket file still exists!"
    echo "  This means cleanup did not work properly."
    ls -la "$SOCKET_PATH"
    rm -f "$SOCKET_PATH"
    exit 1
else
    echo "  ✓ SUCCESS: Socket file was properly cleaned up"
fi

# 7. 测试旧文件清理（模拟 unclean exit）
echo
echo "Step 6: Testing stale file cleanup..."
touch "$SOCKET_PATH"
echo "  Created stale socket file"

./dream --mode receive \
    --input-file "$TEST_INPUT" \
    --status-socket "$SOCKET_PATH" &
DREAM_PID=$!
sleep 2

if [ -S "$SOCKET_PATH" ]; then
    echo "  ✓ Dream cleaned up stale file and created new socket"
else
    echo "  ✗ Failed to recreate socket"
    kill $DREAM_PID 2>/dev/null || true
    exit 1
fi

kill -INT $DREAM_PID
sleep 2

if [ ! -S "$SOCKET_PATH" ]; then
    echo "  ✓ Socket properly cleaned up after second run"
else
    echo "  ✗ Socket not cleaned up"
    rm -f "$SOCKET_PATH"
fi

echo
echo "========================================="
echo "✓ All cleanup tests passed!"
echo "========================================="
