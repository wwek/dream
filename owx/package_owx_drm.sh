#!/bin/bash
#
# OpenWebRX DRM Package Creator
# OpenWebRX DRM 打包工具
# Version: 1.0
# Date: 2025-10-27
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
    echo "   OpenWebRX DRM Package Creator"
    echo "   OpenWebRX DRM 打包工具"
    echo "   Version: 1.0"
    echo "================================================"
    echo ""
}

# Get script directory / 获取脚本目录
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
OUTPUT_DIR="$PROJECT_ROOT/output"
TIMESTAMP=$(date +%Y%m%d%H%M%S)
ARCHIVE_NAME="owx_drm_status_patch_${TIMESTAMP}.tar.gz"
TEMP_DIR=$(mktemp -d)
PACKAGE_DIR="$TEMP_DIR/owx_drm_package"

# Cleanup function / 清理函数
cleanup() {
    if [ -d "$TEMP_DIR" ]; then
        rm -rf "$TEMP_DIR"
    fi
}

# Register cleanup on exit / 注册退出时清理
trap cleanup EXIT

# Create package directory / 创建打包目录
create_package_dir() {
    log_info "Creating temporary directory... / 创建临时目录..."
    mkdir -p "$PACKAGE_DIR"
    log_success "Temporary directory created / 临时目录创建完成: $PACKAGE_DIR"
}

# Copy drm_panel / 复制 drm_panel
copy_drm_panel() {
    log_info "Copying drm_panel... / 复制 drm_panel..."

    if [ -d "$SCRIPT_DIR/drm_panel" ]; then
        cp -r "$SCRIPT_DIR/drm_panel" "$PACKAGE_DIR/"
        log_success "drm_panel copied / drm_panel 已复制"
    else
        log_error "drm_panel directory not found / drm_panel 目录不存在"
        return 1
    fi
}

# Copy drm_status_patch / 复制 drm_status_patch
copy_drm_status_patch() {
    log_info "Copying drm_status_patch... / 复制 drm_status_patch..."

    if [ -d "$SCRIPT_DIR/drm_status_patch" ]; then
        cp -r "$SCRIPT_DIR/drm_status_patch" "$PACKAGE_DIR/"
        log_success "drm_status_patch copied / drm_status_patch 已复制"
    else
        log_error "drm_status_patch directory not found / drm_status_patch 目录不存在"
        return 1
    fi
}

# Copy documentation / 复制文档
copy_documentation() {
    log_info "Copying documentation... / 复制文档..."

    local files_copied=0

    # Copy README.md
    if [ -f "$SCRIPT_DIR/README.md" ]; then
        cp "$SCRIPT_DIR/README.md" "$PACKAGE_DIR/"
        ((files_copied++))
    else
        log_warning "README.md not found / README.md 未找到"
    fi

    # Copy owx_drm_patch_readme_cn.docx
    if [ -f "$SCRIPT_DIR/owx_drm_patch_readme_cn.docx" ]; then
        cp "$SCRIPT_DIR/owx_drm_patch_readme_cn.docx" "$PACKAGE_DIR/"
        ((files_copied++))
    else
        log_warning "owx_drm_patch_readme_cn.docx not found / owx_drm_patch_readme_cn.docx 未找到"
    fi

    if [ $files_copied -gt 0 ]; then
        log_success "Documentation copied ($files_copied files) / 文档已复制 ($files_copied 个文件)"
    else
        log_warning "No documentation files found / 未找到文档文件"
    fi
}

# Copy Dream binaries / 复制 Dream 二进制文件
copy_dream_binaries() {
    log_info "Checking Dream binaries... / 检查 Dream 二进制文件..."

    local dream_files=(
        "dream-amd64"
        "dream-arm64"
        "dream-armv7"
    )

    local found_count=0
    local dream_dir="$PACKAGE_DIR/dream"

    # Check if any Dream binary exists
    for binary in "${dream_files[@]}"; do
        if [ -f "$OUTPUT_DIR/$binary" ]; then
            ((found_count++))
        fi
    done

    if [ $found_count -eq 0 ]; then
        log_warning "No Dream binaries found in output/ / output/ 中未找到 Dream 二进制文件"
        log_info "Skipping Dream binaries / 跳过 Dream 二进制文件"
        return 0
    fi

    # Create dream directory
    mkdir -p "$dream_dir"

    # Copy found binaries
    log_success "Found $found_count Dream binaries / 找到 $found_count 个 Dream 二进制文件"
    for binary in "${dream_files[@]}"; do
        if [ -f "$OUTPUT_DIR/$binary" ]; then
            cp "$OUTPUT_DIR/$binary" "$dream_dir/"
            local size=$(du -h "$OUTPUT_DIR/$binary" | cut -f1)
            echo "  ✓ $binary ($size)"
        fi
    done
}

# Create archive / 创建压缩包
create_archive() {
    log_info "Creating archive... / 创建压缩包..."

    # Ensure output directory exists
    mkdir -p "$OUTPUT_DIR"

    # Create tar.gz
    cd "$TEMP_DIR"
    tar -czf "$ARCHIVE_NAME" -C "$PACKAGE_DIR" .

    # Move to output directory
    mv "$ARCHIVE_NAME" "$OUTPUT_DIR/"

    log_success "Archive created / 压缩包创建完成"
}

# Display results / 显示结果
display_results() {
    local archive_path="$OUTPUT_DIR/$ARCHIVE_NAME"
    local archive_size=$(du -h "$archive_path" | cut -f1)

    echo ""
    echo "================================================"
    log_success "Package created successfully! / 打包成功完成!"
    echo "================================================"
    echo ""
    echo "File / 文件: $archive_path"
    echo "Size / 大小: $archive_size"
    echo ""
    echo "Contents / 内容:"

    # List archive contents
    tar -tzf "$archive_path" | grep -E "^[^/]+/$" | sed 's/\/$//' | while read -r item; do
        if [ "$item" = "dream" ]; then
            local dream_count=$(tar -tzf "$archive_path" | grep "^dream/dream-" | wc -l | tr -d ' ')
            echo "  - $item/ ($dream_count binaries)"
        else
            echo "  - $item/"
        fi
    done

    # List files in root
    tar -tzf "$archive_path" | grep -E "^[^/]+\.(md|docx)$" | while read -r file; do
        echo "  - $file"
    done

    echo ""
    log_info "Extract with / 解压命令: tar -xzf $ARCHIVE_NAME"
    echo ""
}

# Main process / 主流程
main() {
    print_header

    # 1. Create package directory
    create_package_dir

    # 2. Copy drm_panel
    if ! copy_drm_panel; then
        log_error "Failed to copy drm_panel / 复制 drm_panel 失败"
        exit 1
    fi

    # 3. Copy drm_status_patch
    if ! copy_drm_status_patch; then
        log_error "Failed to copy drm_status_patch / 复制 drm_status_patch 失败"
        exit 1
    fi

    # 4. Copy documentation
    copy_documentation

    # 5. Copy Dream binaries (optional)
    copy_dream_binaries

    # 6. Create archive
    if ! create_archive; then
        log_error "Failed to create archive / 创建压缩包失败"
        exit 1
    fi

    # 7. Display results
    display_results
}

# Execute main process / 执行主流程
main "$@"
