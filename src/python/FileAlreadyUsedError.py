class FileAlreadyUsedError(Exception):
    """Exception raised when a file is used more than once."""
    def __init__(self, filename):
        super().__init__(f"The file '{filename}' has already been used.")
        self.filename = filename
