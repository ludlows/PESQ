# 2019-May
# github.com/ludlows
# Python Wrapper for PESQ Score (narrow band and wide band)

import numpy as np
from .cypesq import cypesq, cypesq_retvals, cypesq_error_message as pesq_error_message
from .cypesq import PesqError, InvalidSampleRateError, OutOfMemoryError
from .cypesq import BufferTooShortError, NoUtterancesError

USAGE = """
       Run model on reference ref and degraded deg
       Sample rate (fs) - No default. Must select either 8000 or 16000.
       Note there is narrow band (nb) mode only when sampling rate is 8000Hz.
       """
def pesq(fs, ref, deg, mode, on_error=PesqError.RAISE_EXCEPTION):
    """
    Args:
        ref: numpy 1D array, reference audio signal 
        deg: numpy 1D array, degraded audio signal
        fs:  integer, sampling rate
        mode: 'wb' (wide-band) or 'nb' (narrow-band)
    Returns:
        pesq_score: float, P.862.2 Prediction (MOS-LQO)
    """
    if mode != 'wb' and mode != 'nb':
        print(USAGE)
        raise ValueError("mode should be either 'nb' or 'wb'")

    if fs != 8000 and fs != 16000:
        print(USAGE)
        raise ValueError("fs (sampling frequency) should be either 8000 or 16000")

    if fs == 8000 and mode == 'wb':
        print(USAGE)
        raise ValueError("no wide band mode if fs = 8000")

    maxval = max(np.max(np.abs(ref/1.0)), np.max(np.abs(deg/1.0)))

    if mode == 'wb':
        mode_code = 1
    else:
        mode_code = 0
    
    if on_error == PesqError.RETURN_VALUES:
        return cypesq_retvals(
            fs,
            (ref/maxval).astype(np.float32),
            (deg/maxval).astype(np.float32),
            mode_code
        )
    else:
        return cypesq(
            fs,
            (ref/maxval).astype(np.float32),
            (deg/maxval).astype(np.float32),
            mode_code
        )
