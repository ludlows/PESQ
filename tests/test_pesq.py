import pytest
import numpy as np
import scipy.io.wavfile

from pathlib import Path

from pesq import pesq, NoUtterancesError, PesqError


def test():
    data_dir = Path(__file__).parent.parent / 'audio'
    ref_path = data_dir / 'speech.wav'
    deg_path = data_dir / 'speech_bab_0dB.wav'

    sample_rate, ref = scipy.io.wavfile.read(ref_path)
    sample_rate, deg = scipy.io.wavfile.read(deg_path)

    score = pesq(ref=ref, deg=deg, fs=sample_rate, mode='wb')

    assert score == 1.0832337141036987, score

    score = pesq(ref=ref, deg=deg, fs=sample_rate, mode='nb')

    assert score == 1.6072081327438354, score
    return score


def test_no_utterances_nb_mode():
    sample_rate = 8000
    silent_ref = np.zeros(sample_rate)
    deg = np.random.randn(sample_rate)

    with pytest.raises(NoUtterancesError) as e:
        pesq(ref=silent_ref, deg=deg, fs=sample_rate, mode='nb')

    score = pesq(ref=silent_ref, deg=deg, fs=sample_rate, mode='nb',
                 on_error=PesqError.RETURN_VALUES)

    assert score == PesqError.NO_UTTERANCES_DETECTED, score
    return score


def test_no_utterances_wb_mode():
    sample_rate = 16000
    silent_ref = np.zeros(sample_rate)
    deg = np.random.randn(sample_rate)

    with pytest.raises(NoUtterancesError) as e:
        pesq(ref=silent_ref, deg=deg, fs=sample_rate, mode='wb')

    score = pesq(ref=silent_ref, deg=deg, fs=sample_rate, mode='wb',
                 on_error=PesqError.RETURN_VALUES)

    assert score == PesqError.NO_UTTERANCES_DETECTED, score
    return score


# if __name__ == "__main__":
#     print(test())
#     print(test_no_utterances_wb_mode())
#     print(test_no_utterances_nb_mode())
