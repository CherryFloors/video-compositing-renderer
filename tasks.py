import sys
import os
from pathlib import Path

import invoke
from invoke.context import Context


WINDOWS = invoke.terminals.WINDOWS
USE_PTY = not WINDOWS


@invoke.task
def build(context: Context) -> None:
    """Builds the package"""

    cmd = "python -m build . --wheel"
    context.run(cmd, echo=True, pty=USE_PTY)


@invoke.task
def debug_build(context: Context) -> None:
    """Builds the package in debug mode locally"""

    cmd = "DEBUG=1 pip install -e ."
    context.run(cmd, echo=True, pty=USE_PTY)


@invoke.task
def fmt(context: Context) -> None:
    """Formats Python code with black."""

    clang_style = '"{BasedOnStyle: llvm, IndentWidth: 4, ColumnLimit: 120, AlignAfterOpenBracket: true}"'
    context.run(f"clang-format --style={clang_style} -i c/*.[ch] ", echo=True, pty=USE_PTY)
    context.run("black --verbose .", echo=True, pty=USE_PTY)


@invoke.task
def pylint_errors(context: Context) -> None:
    """Prints out the pylint errors"""

    context.run("pylint --disable=W,R,C ./src", warn=True, echo=True, pty=USE_PTY)
    context.run("pylint --disable=W,R,C ./tests", warn=True, echo=True, pty=USE_PTY)


@invoke.task
def validate(context: Context) -> None:
    """Validate the project"""

    context.run("pyflakes ./src", echo=True, pty=USE_PTY)
    context.run("pyflakes ./tests", echo=True, pty=USE_PTY)
    context.run("black --check --diff --verbose ./src", echo=True, pty=USE_PTY)
    context.run("black --check --diff --verbose ./tests", echo=True, pty=USE_PTY)

    def pylint(path) -> None:
        """Runs pylint on path."""

        pylint_ret = context.run(f"pylint {path}", warn=True, echo=True, pty=USE_PTY).exited
        if pylint_ret and (1 & pylint_ret or 2 & pylint_ret or 32 & pylint_ret):
            print(f"pylint exit code: {pylint_ret}")
            sys.exit(pylint_ret)

    pylint("./src")
    pylint("./tests")

    context.run("mypy --install-types --non-interactive ./src", echo=True, pty=USE_PTY)
    context.run("mypy --install-types --non-interactive ./tests", echo=True, pty=USE_PTY)


@invoke.task
def test(context: Context) -> None:
    """Runs unit tests."""

    test_cmd = "pytest tests --ff --verbose --cov-report=xml --cov=vcr"
    coverage_cmd = "coverage report --fail-under=80 -m"

    context.run(test_cmd, echo=True, pty=USE_PTY)
    context.run(coverage_cmd, echo=True, pty=USE_PTY)


@invoke.task
def set_version(context: Context) -> None:
    """Sets __version__ in __init__.py file."""

    # VERSION will be exported in CI on commits to master
    __version__ = os.getenv("CI_COMMIT_TAG", "0.0.0rc0")
    dunderinitpy = Path("src/vcr/__init__.py")
    contents = dunderinitpy.read_text().splitlines()

    version_line_finder = (i for i, line in enumerate(contents) if line == '__version__ = "0.0.0rc0"')
    version_line = next(version_line_finder)

    contents[version_line] = f'__version__ = "{__version__}"'
    dunderinitpy.write_text("\n".join(contents) + "\n")
    print(f"Version set to '{__version__}' in '{dunderinitpy}'")
