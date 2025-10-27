from pycsdr.modules import ExecModule
from pycsdr.types import Format
import uuid
import os


class DrmModule(ExecModule):
    def __init__(self):
        # 每个实例使用唯一 socket 路径（多用户支持）
        self.instance_id = str(uuid.uuid4())[:8]
        self.socket_path = f"/tmp/dream_status_{self.instance_id}.sock"

        # 启动前清理可能存在的旧 socket
        if os.path.exists(self.socket_path):
            try:
                os.unlink(self.socket_path)
            except OSError:
                pass

        super().__init__(
            Format.COMPLEX_SHORT,
            Format.SHORT,
            ["dream", "-c", "6", "--sigsrate", "48000", "--audsrate", "48000",
             "-I", "-", "-O", "-", "--status-socket", self.socket_path]
        )

    def getSocketPath(self):
        return self.socket_path

    def stop(self):
        """停止模块并清理 socket 文件"""
        super().stop()
        # 清理 socket 文件
        if os.path.exists(self.socket_path):
            try:
                os.unlink(self.socket_path)
            except OSError:
                pass
