from pathlib import Path

import scipy.io.wavfile

from pesq import pesq


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
