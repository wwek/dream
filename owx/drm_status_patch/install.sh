#!/bin/bash
#
# DRM Status Bug Fix Patch - Auto Installation Script
# DRM 状态补丁 - 自动安装脚本
# Version: 1.0
# Date: 2025-10-26
#

set -e

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
    echo "   DRM Status Bug Fix Patch - Installation Script"
    echo "   DRM 状态补丁 - 安装脚本"
    echo "   Version: 1.0"
    echo "================================================"
    echo ""
}

# Detect OpenWebRX installation path / 检测 OpenWebRX 安装路径
detect_openwebrx_path() {
    log_info "Detecting OpenWebRX installation path... / 检测 OpenWebRX 安装路径..."

    # 尝试多个可能的路径
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

# Validate target path / 验证目标路径
validate_path() {
    if [ ! -d "$1" ]; then
        log_error "Path does not exist / 路径不存在: $1"
        return 1
    fi

    # Check key files exist / 检查关键文件是否存在
    local key_files=(
        "$1/csdr/chain"
        "$1/csdr/module"
        "$1/owrx"
    )

    for file in "${key_files[@]}"; do
        if [ ! -d "$file" ]; then
            log_error "Key directory missing / 关键目录不存在: $file"
            log_error "This may not be a valid OpenWebRX installation / 这可能不是一个有效的 OpenWebRX 安装路径"
            return 1
        fi
    done

    return 0
}

# Create backup / 创建备份
create_backup() {
    log_info "Creating backup... / 创建备份..."

    BACKUP_DIR="$OPENWEBRX_PATH/backup_$(date +%Y%m%d_%H%M%S)"
    mkdir -p "$BACKUP_DIR"

    # Files to backup / 备份要修改的文件
    local files_to_backup=(
        "csdr/chain/drm.py"
        "csdr/module/drm.py"
        "owrx/drm.py"
    )

    for file in "${files_to_backup[@]}"; do
        if [ -f "$OPENWEBRX_PATH/$file" ]; then
            local backup_path="$BACKUP_DIR/$file"
            mkdir -p "$(dirname "$backup_path")"
            cp "$OPENWEBRX_PATH/$file" "$backup_path"
            log_success "Backed up / 已备份: $file"
        else
            log_warning "File not found, skipping / 文件不存在，跳过备份: $file"
        fi
    done

    # Save backup path to file / 保存备份路径到文件
    echo "$BACKUP_DIR" > "$OPENWEBRX_PATH/.drm_patch_backup"
    log_success "Backup completed / 备份完成: $BACKUP_DIR"
}

# Apply patch / 应用补丁
apply_patch() {
    log_info "Applying patch files... / 应用补丁文件..."

    local patch_files=(
        "csdr/chain/drm.py"
        "csdr/module/drm.py"
        "owrx/drm.py"
    )

    for file in "${patch_files[@]}"; do
        if [ -f "$file" ]; then
            local target="$OPENWEBRX_PATH/$file"
            mkdir -p "$(dirname "$target")"
            cp "$file" "$target"
            log_success "Installed / 已安装: $file"
        else
            log_error "Patch file not found / 补丁文件不存在: $file"
            return 1
        fi
    done

    log_success "Patch applied successfully / 补丁应用完成"
}

# Verify installation / 验证安装
verify_installation() {
    log_info "Verifying patch installation... / 验证补丁安装..."

    local checks=(
        "grep -q 'threading.Lock' '$OPENWEBRX_PATH/csdr/chain/drm.py'"
        "grep -q 'drm_module.stop' '$OPENWEBRX_PATH/csdr/chain/drm.py'"
        "grep -q 'os.unlink' '$OPENWEBRX_PATH/csdr/module/drm.py'"
        "grep -q 'buffer = b' '$OPENWEBRX_PATH/owrx/drm.py'"
    )

    local all_passed=true
    for check in "${checks[@]}"; do
        if eval "$check" 2>/dev/null; then
            log_success "Check passed / 验证通过: $(echo "$check" | cut -d"'" -f2)"
        else
            log_error "Check failed / 验证失败: $(echo "$check" | cut -d"'" -f2)"
            all_passed=false
        fi
    done

    if [ "$all_passed" = true ]; then
        log_success "All checks passed / 所有验证通过"
        return 0
    else
        log_error "Some checks failed, please verify installation / 部分验证失败，请检查安装"
        return 1
    fi
}

# Restart service / 重启服务
restart_service() {
    log_info "Attempting to restart OpenWebRX service... / 尝试重启 OpenWebRX 服务..."

    # Detect service type / 检测服务类型
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

# Main installation process / 主安装流程
main() {
    print_header

    # Change to script directory / 切换到脚本所在目录
    cd "$(dirname "$0")"

    # 1. Detect or ask for OpenWebRX path / 检测或询问 OpenWebRX 路径
    if ! detect_openwebrx_path; then
        log_warning "Could not auto-detect OpenWebRX path / 未能自动检测 OpenWebRX 路径"
        echo -n "Please enter OpenWebRX installation path / 请输入 OpenWebRX 安装路径: "
        read -r OPENWEBRX_PATH

        if ! validate_path "$OPENWEBRX_PATH"; then
            log_error "Invalid installation path / 无效的安装路径"
            exit 1
        fi
    fi

    # 2. Confirm installation / 确认安装
    echo ""
    log_info "Will install patch to / 即将在以下路径安装补丁:"
    echo "  $OPENWEBRX_PATH"
    echo ""
    echo -n "Continue? / 是否继续? [y/N] "
    read -r confirm

    if [[ ! "$confirm" =~ ^[Yy]$ ]]; then
        log_info "Installation cancelled / 安装已取消"
        exit 0
    fi

    # 3. Create backup / 创建备份
    create_backup

    # 4. Apply patch / 应用补丁
    if ! apply_patch; then
        log_error "Patch application failed / 补丁应用失败"
        echo ""
        log_info "You can rollback using / 可以使用以下命令回滚:"
        echo "  ./uninstall.sh"
        exit 1
    fi

    # 5. Verify installation / 验证安装
    if ! verify_installation; then
        log_warning "Some checks did not pass, but patch is installed / 验证未完全通过，但补丁已安装"
    fi

    # 6. Restart service / 重启服务
    echo ""
    restart_service

    # 7. Completion message / 完成提示
    echo ""
    echo "================================================"
    log_success "Patch installation completed! / 补丁安装完成!"
    echo "================================================"
    echo ""
    log_info "Backup location / 备份位置: $BACKUP_DIR"
    log_info "To rollback, run / 如需回滚，请运行: ./uninstall.sh"
    echo ""
    log_info "Fixed issues / 修复内容:"
    echo "  ✓ Thread safety / 线程安全问题"
    echo "  ✓ Resource leaks / 资源泄漏"
    echo "  ✓ Socket file cleanup / Socket 文件清理"
    echo "  ✓ UTF-8 encoding / UTF-8 编码处理"
    echo "  ✓ Exception handling / 异常处理优化"
    echo ""
}

# Execute main process / 执行主流程
main "$@"
