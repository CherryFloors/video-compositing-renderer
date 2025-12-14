from enum import Enum
from dataclasses import dataclass

from vcr._libvcr import _enqueue_program, _queue_count, _clear_queue


class EnqueueCode(int, Enum):
    """EnqueueCode"""

    SUCCESS = 0
    FAIL_QUEUE_FULL = 1
    FAIL_URL_LENGTH = 2


@dataclass
class Program:
    """Program"""

    url: str

    @property
    def c_args(self) -> tuple:
        return (self.url,)


def enqueue_program(program: Program) -> EnqueueCode:
    """enqueue_program"""
    return EnqueueCode(_enqueue_program(*program.c_args))


def queue_count() -> int:
    """queue_count"""
    return _queue_count()


def clear_queue() -> int:
    """clear_queue"""
    return _clear_queue()
