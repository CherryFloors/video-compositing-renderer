"""Test vcr.__main__"""

from vcr.__main__ import cli


class TestCli:
    """TestDeleteMe"""

    @staticmethod
    def test_cli() -> None:
        """test_cli"""

        assert cli() == 0
