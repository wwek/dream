# DRM Status Patch

## Version Information
- **Patch Version**: 1.0
- **Release Date**: 2025-10-26
- **Compatible With**: OpenWebRX+ (versions with DRM support)

## Patch Contents
- Add DRM status information to WebSocket messages


## Installation

### Method 1: Automatic Installation (Recommended)
```bash
# 1. Extract the patch
tar -xzf drm_status_patch.tar.gz
cd drm_status_patch

# 2. Run the installation script
./install.sh

# The script will automatically:
# - Detect OpenWebRX path
# - Create backups
# - Apply patches
# - Verify installation
# - Restart service
```

### Method 2: Manual Installation
Manually replace the following files:
- `csdr/chain/drm.py`
- `csdr/module/drm.py`
- `owrx/drm.py`

## Patch Verification

### Check File Changes
```bash
# Verify the fixes are present
grep -n "threading.Lock" csdr/chain/drm.py        # Should be found
grep -n "drm_module.stop" csdr/chain/drm.py       # Should be found
grep -n "os.unlink" csdr/module/drm.py             # Should be found
grep -n "buffer = b\"\"" owrx/drm.py               # Should be found
```

### Runtime Verification
1. Start OpenWebRX and switch to DRM mode
2. Open browser developer tools to check WebSocket messages
3. You should see `{"type": "drm_status", "value": {...}}` messages
4. After stopping DRM, check `/tmp/` directory - no `dream_status_*.sock` files should remain

## Uninstallation/Rollback

If issues occur, you can quickly rollback:
```bash
cd drm_status_patch
./uninstall.sh

# The script will automatically:
# - Locate backups
# - Restore original files
# - Verify rollback
# - Restart service
```

## Technical Details

### Line Changes Summary
- `csdr/chain/drm.py`: +5 lines (add threading import and lock logic)
- `csdr/module/drm.py`: +14 lines (add socket cleanup)
- `owrx/drm.py`: +4 lines (improve buffering and exception handling)

### Dependencies
- Python 3.6+
- threading module (standard library)
- os module (standard library)

### Compatibility
- ✅ Backward compatible: doesn't break existing API
- ✅ Multi-user safe: independent socket per instance
- ✅ Thread-safe: complete concurrency protection

## FAQ

**Q: Does the patch affect performance?**
A: No. Lock overhead is negligible (<1μs), socket cleanup only runs on start/stop.

**Q: Do I need to recompile?**
A: No. This is pure Python code modification.

**Q: Will it affect other demodulation modes?**
A: No. Changes only affect DRM mode.

**Q: Can I use it with other OpenWebRX forks?**
A: Yes, but verify file paths and code structure match.

## Support

If you encounter issues, please provide:
1. OpenWebRX version
2. Python version
3. Error logs (journalctl -u openwebrx -n 100)
4. Patch application status

## License

This patch follows the original OpenWebRX project license.
