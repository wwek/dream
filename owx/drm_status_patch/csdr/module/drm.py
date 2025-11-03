from csdr.module import PopenModule
from pycsdr.types import Format
from owrx.config.core import CoreConfig
import os


class DrmModule(PopenModule):
    def __init__(self):
        # Use id(self) for unique socket path per instance, same as direwolf pattern
        self.socket_path = "{tmp_dir}/dream_status_{myid}.sock".format(
            tmp_dir=CoreConfig().get_temporary_directory(),
            myid=id(self)
        )

        # Clean up old socket if exists
        if os.path.exists(self.socket_path):
            try:
                os.unlink(self.socket_path)
            except OSError:
                pass

        super().__init__()

    def getCommand(self):
        return ["dream", "-c", "6", "--sigsrate", "48000", "--audsrate", "48000",
                "-I", "-", "-O", "-", "--agc", "100", "--status-socket", self.socket_path]

    def getInputFormat(self) -> Format:
        return Format.COMPLEX_SHORT

    def getOutputFormat(self) -> Format:
        return Format.SHORT

    def getSocketPath(self):
        return self.socket_path

    def stop(self):
        """Stop module and clean up socket"""
        # PopenModule.stop() handles process termination
        super().stop()
        # Clean up socket file
        if os.path.exists(self.socket_path):
            try:
                os.unlink(self.socket_path)
            except OSError:
                pass
