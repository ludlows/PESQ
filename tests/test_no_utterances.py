import pytest
import numpy as np
from pesq import pesq, PESQError


def test_no_utterances():
    SAMPLE_RATE = 16000
    silent_ref = np.zeros(16000)
    deg = np.random.randn(16000)

    with pytest.raises(PESQError) as e:
        pesq(SAMPLE_RATE, silent_ref, deg, 'wb')
        assert str(e) == 'No utterances detected'
