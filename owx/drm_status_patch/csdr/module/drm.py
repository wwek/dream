from pycsdr.modules import ExecModule
from pycsdr.types import Format
import os


class DrmModule(ExecModule):
    def __init__(self):
        # Use id(self) for unique socket path per instance
        self.instance_id = id(self)
        self.socket_path = f"/tmp/dream_status_{self.instance_id}.sock"

        # Clean up old socket if exists
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
        """Stop module and clean up socket"""
        super().stop()
        if os.path.exists(self.socket_path):
            try:
                os.unlink(self.socket_path)
            except OSError:
                pass
