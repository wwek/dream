#!/usr/bin/env python3
"""
DRM Status Test Client

Connects to dream Unix Domain Socket and displays real-time status updates.
Supports both simple and detailed display modes.

Usage: python3 test_status.py [--simple] [socket_path]
"""

import socket
import sys
import json
import time
import glob
from datetime import datetime

def find_socket():
    """Find dream socket in /tmp"""
    sockets = glob.glob("/tmp/dream_status_*.sock")
    return sockets[0] if sockets else None

def connect_to_socket(socket_path):
    """Connect to Unix Domain Socket"""
    try:
        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        sock.connect(socket_path)
        return sock
    except Exception as e:
        print(f"❌ Connection failed: {e}")
        return None

def format_status(status_int):
    """Convert status integer to readable string"""
    status_map = {
        0: "NOT_PRESENT",
        1: "CRC_ERROR",
        2: "DATA_ERROR",
        3: "RX_OK"
    }
    return status_map.get(status_int, f"UNKNOWN({status_int})")

def display_simple(data):
    """Simple display mode"""
    print(f"{'='*60}")
    print(f"Time: {data.get('timestamp', 'N/A')}")
    print(f"SNR: {data.get('snr', 'N/A')} dB")
    print(f"MSC Status: {data.get('msc_status', 0)} (0=NONE, 1=CRC_ERR, 2=DATA_ERR, 3=OK)")

    # Show all status
    print(f"Status: IO={data.get('io_status',0)} Time={data.get('time_sync',0)} "
          f"Freq={data.get('freq_sync',0)} FAC={data.get('fac_status',0)} "
          f"SDC={data.get('sdc_status',0)} MSC={data.get('msc_status',0)}")

    # Services
    if 'services' in data and data['services']:
        print(f"Services: {data['services']}")

    print()

def display_detailed(data):
    """Detailed display mode"""
    timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
    print(f"\n{'='*80}")
    print(f"⏰ {timestamp}")
    print(f"{'='*80}")

    # Basic info
    print(f"\n📡 SIGNAL QUALITY:")
    print(f"   SNR: {data.get('snr', 'N/A')} dB")
    print(f"   Doppler: {data.get('doppler', 'N/A')} Hz")
    print(f"   Delay: {data.get('delay', 'N/A')} ms")
    print(f"   Sample Rate Offset: {data.get('sample_rate_offset', 'N/A')} Hz")

    # Reception status
    print(f"\n🔵 STATUS LIGHTS:")
    io_status = format_status(data.get('io_status', 0))
    time_sync = format_status(data.get('time_sync', 0))
    freq_sync = format_status(data.get('freq_sync', 0))
    fac_status = format_status(data.get('fac_status', 0))
    sdc_status = format_status(data.get('sdc_status', 0))
    msc_status = format_status(data.get('msc_status', 0))

    print(f"   IO:        {io_status}")
    print(f"   Time Sync: {time_sync}")
    print(f"   Freq Sync: {freq_sync}")
    print(f"   FAC:       {fac_status}")
    print(f"   SDC:       {sdc_status}")
    print(f"   MSC:       {msc_status}")

    # DRM mode info
    print(f"\n📻 DRM MODE:")
    print(f"   Robustness: Mode {data.get('robustness_mode', 'N/A')}")
    print(f"   Spectrum: {data.get('spectrum_occupancy', 'N/A')}")
    print(f"   Bandwidth: {data.get('bandwidth', 'N/A')} kHz")
    print(f"   Interleaver: {data.get('interleaver_depth', 'N/A')}")

    # Service info
    if 'services' in data and data['services']:
        print(f"\n🎧 SERVICES:")
        for idx, svc in enumerate(data['services']):
            # Handle both dict and string cases
            if isinstance(svc, dict):
                print(f"   [{idx}] {svc.get('label', 'N/A')}")
                print(f"       Audio Codec: {svc.get('audio_codec', 'N/A')}")
                print(f"       Bitrate: {svc.get('bitrate', 'N/A')} kbps")
                print(f"       Language: {svc.get('language', 'N/A')}")
            else:
                print(f"   [{idx}] {svc}")

    # Text message
    if 'text_message' in data and data['text_message']:
        print(f"\n💬 TEXT MESSAGE:")
        print(f"   {data['text_message']}")

def parse_and_display(json_str, simple_mode=False):
    """Parse JSON and display formatted status"""
    try:
        data = json.loads(json_str)

        if simple_mode:
            display_simple(data)
        else:
            display_detailed(data)

        return True

    except json.JSONDecodeError as e:
        print(f"❌ JSON Parse Error: {e}")
        if simple_mode:
            print(f"Raw: {json_str[:100]}...")
        else:
            print(f"Raw data: {json_str[:200]}")
        return False
    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def main():
    # Parse arguments
    simple_mode = False
    socket_path = None

    i = 1
    while i < len(sys.argv):
        arg = sys.argv[i]
        if arg == '--simple' or arg == '-s':
            simple_mode = True
        elif not socket_path and not arg.startswith('-'):
            socket_path = arg
        else:
            print(f"Unknown argument: {arg}")
            print("Usage: python3 test_status.py [--simple] [socket_path]")
            return 1
        i += 1

    # Find socket if not specified
    if not socket_path:
        socket_path = find_socket()
        if not socket_path:
            print("❌ No dream socket found in /tmp/")
            print("Usage: python3 test_status.py [--simple] [socket_path]")
            print("\nStart dream first with:")
            print('  ./dream -f "test_data/drm-bbc XHE-AAC drm iq.rec"')
            return 1

    print(f"🔍 Attempting to connect to: {socket_path}")

    # Connect to socket
    sock = connect_to_socket(socket_path)
    if not sock:
        return 1

    if simple_mode:
        print("✅ Connected! Simple mode. Receiving updates...\n")
    else:
        print("✅ Connected! Detailed mode. Receiving updates (Ctrl+C to exit)...\n")

    # Receive and display updates
    buffer = ""
    try:
        while True:
            try:
                # Receive data
                data = sock.recv(4096)
                if not data:
                    print("❌ Connection closed by server")
                    break

                # Decode and accumulate
                buffer += data.decode('utf-8', errors='ignore')

                # Process complete JSON messages (separated by newlines)
                while '\n' in buffer:
                    line, buffer = buffer.split('\n', 1)
                    line = line.strip()
                    if line:
                        parse_and_display(line, simple_mode)

            except socket.timeout:
                continue
            except KeyboardInterrupt:
                print("\n\n👋 Exiting...")
                break

    finally:
        sock.close()
        print("✅ Socket closed")

    return 0

if __name__ == "__main__":
    sys.exit(main())