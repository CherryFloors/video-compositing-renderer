"""Tests for the libvcr and _libvcr modules"""

from vcr._libvcr import _enqueue_program, _test_dequeue_program, _clear_queue, _queue_count


class TestCLibVcr:
    """Tests for _libvcr c extension API"""

    @staticmethod
    def test_queue() -> None:
        """test the queue from the private c extension API"""

        assert _clear_queue() == 0
        assert _queue_count() == 0

        for i in range(1000):
            print(i)
            queue_code = _enqueue_program(f"MOVIE: {i}")

            if i < 100:
                assert queue_code == 0
            else:
                assert queue_code == 1

        assert _queue_count() == 100

        for i in range(1000):

            url = _test_dequeue_program()

            if i < 100:
                assert url == f"MOVIE: {i}"
            else:
                assert url == ""

        long_string = "a" * 512
        long_queue_code = _enqueue_program(long_string)
        assert long_queue_code == 2

        real_url = "http://127.0.0.1:8000/Items/4b8e6c1a-7449-4864-9c76-5cac9e1d6565/Download?api_key=600540be-270e-44d2-a47d-027b47eb5321"
        real_queue_code = _enqueue_program(real_url)
        assert real_queue_code == 0
        assert _test_dequeue_program() == real_url

        second_real_queue_code = _enqueue_program(real_url)
        assert second_real_queue_code == 0
        assert _queue_count() == 1
        assert _clear_queue() == 0
        assert _queue_count() == 0
