from pathlib import Path

import soundfile

from pypesq import pypesq


def test():
    data_dir = Path(__file__).parent.parent / 'audio'
    ref_path = data_dir / 'speech.wav'
    deg_path = data_dir / 'speech_bab_0dB.wav'

    ref, sample_rate = soundfile.read(ref_path)
    deg, sample_rate = soundfile.read(deg_path)

    score = pypesq(ref=ref, deg=deg, fs=sample_rate, mode='wb')

    assert score == 1.0832337141036987, score

    score = pypesq(ref=ref, deg=deg, fs=sample_rate, mode='nb')

    assert score == 1.6072081327438354, score
