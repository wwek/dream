#!/bin/bash
#
# DRM Status Bug Fix Patch - Uninstall/Rollback Script
# DRM 状态补丁 - 卸载/回滚脚本
# Version: 1.0
# Date: 2025-10-26
#

set -e

# Parse command line arguments / 解析命令行参数
AUTO_CONFIRM=false
while getopts "yh" opt; do
    case $opt in
        y)
            AUTO_CONFIRM=true
            ;;
        h)
            echo "Usage: $0 [-y] [-h]"
            echo "  -y    Auto-confirm uninstallation (skip confirmation prompts)"
            echo "        自动确认卸载（跳过确认提示）"
            echo "  -h    Show this help message"
            echo "        显示此帮助信息"
            exit 0
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            echo "Use -h for help"
            exit 1
            ;;
    esac
done

# Color output / 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions / 日志函数
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Print header / 打印标题
print_header() {
    echo ""
    echo "================================================"
    echo "   DRM Status Patch - Uninstall/Rollback Script"
    echo "   DRM 状态补丁 - 卸载/回滚脚本"
    echo "   Version: 1.0"
    echo "================================================"
    echo ""
}

# Detect OpenWebRX path / 检测 OpenWebRX 路径
detect_openwebrx_path() {
    log_info "Detecting OpenWebRX installation path... / 检测 OpenWebRX 安装路径..."

    local possible_paths=(
        "/usr/lib/python3/dist-packages"
        "/opt/openwebrx"
        "$(pip3 show openwebrx 2>/dev/null | grep Location | awk '{print $2}')"
        "/home/openwebrx"
        "$HOME/openwebrx"
    )

    for path in "${possible_paths[@]}"; do
        if [ -n "$path" ] && [ -d "$path/owrx" ] && [ -d "$path/csdr" ]; then
            OPENWEBRX_PATH="$path"
            log_success "Found OpenWebRX: $OPENWEBRX_PATH / 找到 OpenWebRX: $OPENWEBRX_PATH"
            return 0
        fi
    done

    return 1
}

# Find backup / 查找备份
find_backup() {
    log_info "Searching for backup files... / 查找备份文件..."

    # Read backup path file / 读取备份路径文件
    if [ -f "$OPENWEBRX_PATH/.drm_patch_backup" ]; then
        BACKUP_DIR=$(cat "$OPENWEBRX_PATH/.drm_patch_backup")
        if [ -d "$BACKUP_DIR" ]; then
            log_success "Found backup: $BACKUP_DIR / 找到备份: $BACKUP_DIR"
            return 0
        else
            log_warning "Backup path recorded but directory not found / 备份路径记录存在但目录不存在"
        fi
    fi

    # Search for recent backup directory / 搜索最近的备份目录
    local latest_backup=$(find "$OPENWEBRX_PATH" -maxdepth 1 -type d -name "backup_*" | sort -r | head -1)
    if [ -n "$latest_backup" ]; then
        BACKUP_DIR="$latest_backup"
        log_success "Found recent backup: $BACKUP_DIR / 找到最近备份: $BACKUP_DIR"
        return 0
    fi

    log_error "No backup found / 未找到备份文件"
    return 1
}

# Restore backup / 恢复备份
restore_backup() {
    log_info "Restoring backup files... / 恢复备份文件..."

    local files_to_restore=(
        "csdr/chain/drm.py"
        "csdr/module/drm.py"
        "owrx/drm.py"
    )

    local restored_count=0
    for file in "${files_to_restore[@]}"; do
        if [ -f "$BACKUP_DIR/$file" ]; then
            cp "$BACKUP_DIR/$file" "$OPENWEBRX_PATH/$file"
            log_success "Restored / 已恢复: $file"
            ((restored_count++))
        else
            log_warning "Not found in backup / 备份中不存在: $file"
        fi
    done

    if [ $restored_count -eq 0 ]; then
        log_error "No files restored / 没有文件被恢复"
        return 1
    fi

    log_success "Restore completed ($restored_count files) / 恢复完成 ($restored_count 个文件)"
}

# Cleanup patch markers / 清理补丁标记
cleanup_markers() {
    log_info "Cleaning up patch markers... / 清理补丁标记..."

    if [ -f "$OPENWEBRX_PATH/.drm_patch_backup" ]; then
        rm "$OPENWEBRX_PATH/.drm_patch_backup"
        log_success "Patch markers cleaned / 已清理补丁标记"
    fi
}

# Verify rollback / 验证回滚
verify_rollback() {
    log_info "Verifying rollback... / 验证回滚..."

    # Check if patch features removed / 检查补丁特征是否已移除
    local checks=(
        "grep -q 'threading.Lock' '$OPENWEBRX_PATH/csdr/chain/drm.py'"
    )

    local has_patch=false
    for check in "${checks[@]}"; do
        if eval "$check" 2>/dev/null; then
            has_patch=true
            break
        fi
    done

    if [ "$has_patch" = true ]; then
        log_warning "Patch features still present, may not be fully rolled back / 补丁特征仍然存在，可能未完全回滚"
        return 1
    else
        log_success "Verification passed, patch completely removed / 验证通过，补丁已完全移除"
        return 0
    fi
}

# Restart service / 重启服务
restart_service() {
    log_info "Attempting to restart OpenWebRX service... / 尝试重启 OpenWebRX 服务..."

    if systemctl is-active --quiet openwebrx 2>/dev/null; then
        log_info "Detected systemd service / 检测到 systemd 服务"
        if [ "$EUID" -eq 0 ]; then
            systemctl restart openwebrx
            log_success "Service restarted / 服务已重启"
        else
            log_warning "Root permission required to restart service / 需要 root 权限重启服务"
            echo "Please run manually / 请手动执行: sudo systemctl restart openwebrx"
        fi
    else
        log_warning "systemd service not detected / 未检测到 systemd 服务"
        log_info "Please restart OpenWebRX manually / 请手动重启 OpenWebRX"
    fi
}

# Ask to delete backup / 询问是否删除备份
ask_delete_backup() {
    echo ""

    if [ "$AUTO_CONFIRM" = false ]; then
        echo -n "Delete backup directory? / 是否删除备份目录? [y/N] "
        read -r confirm

        if [[ "$confirm" =~ ^[Yy]$ ]]; then
            if [ -d "$BACKUP_DIR" ]; then
                rm -rf "$BACKUP_DIR"
                log_success "Backup deleted / 备份已删除: $BACKUP_DIR"
            fi
        else
            log_info "Backup kept / 备份已保留: $BACKUP_DIR"
        fi
    else
        log_info "Keeping backup (auto-confirm mode) / 保留备份（自动确认模式）: $BACKUP_DIR"
    fi
}

# Main uninstall process / 主卸载流程
main() {
    print_header

    # 1. Detect OpenWebRX path / 检测 OpenWebRX 路径
    if ! detect_openwebrx_path; then
        log_warning "Could not auto-detect OpenWebRX path / 未能自动检测 OpenWebRX 路径"
        echo -n "Please enter OpenWebRX installation path / 请输入 OpenWebRX 安装路径: "
        read -r OPENWEBRX_PATH

        if [ ! -d "$OPENWEBRX_PATH" ]; then
            log_error "Path does not exist / 路径不存在: $OPENWEBRX_PATH"
            exit 1
        fi
    fi

    # 2. Find backup / 查找备份
    if ! find_backup; then
        log_error "Cannot find backup, unable to rollback / 无法找到备份，无法执行回滚"
        echo ""
        log_info "If you still want to remove the patch, please restore files manually / 如果您仍想移除补丁，请手动恢复文件"
        exit 1
    fi

    # 3. Confirm rollback / 确认回滚
    echo ""
    log_info "Will rollback from the following backup / 即将从以下备份回滚:"
    echo "  Backup / 备份: $BACKUP_DIR"
    echo "  Target / 目标: $OPENWEBRX_PATH"
    echo ""

    if [ "$AUTO_CONFIRM" = false ]; then
        echo -n "Continue? / 是否继续? [y/N] "
        read -r confirm

        if [[ ! "$confirm" =~ ^[Yy]$ ]]; then
            log_info "Rollback cancelled / 回滚已取消"
            exit 0
        fi
    else
        log_info "Auto-confirming rollback (-y flag) / 自动确认回滚 (-y 参数)"
    fi

    # 4. Restore backup / 恢复备份
    if ! restore_backup; then
        log_error "Backup restore failed / 备份恢复失败"
        exit 1
    fi

    # 5. Cleanup markers / 清理标记
    cleanup_markers

    # 6. Verify rollback / 验证回滚
    verify_rollback || log_warning "Please check file contents manually / 请手动检查文件内容"

    # 7. Restart service / 重启服务
    echo ""
    restart_service

    # 8. Ask to delete backup / 询问删除备份
    ask_delete_backup

    # 9. Completion message / 完成提示
    echo ""
    echo "================================================"
    log_success "Rollback completed! / 回滚完成!"
    echo "================================================"
    echo ""
    log_info "Patch removed, system restored to pre-installation state / 补丁已移除，系统已恢复到安装前状态"
    echo ""
}

# Execute main process / 执行主流程
main "$@"
