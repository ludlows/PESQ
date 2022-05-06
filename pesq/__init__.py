# 2019-May
# github.com/ludlows
# Python Wrapper for PESQ Score (narrowband and wideband)

import numpy as np
from .cypesq import cypesq, cypesq_retvals, cypesq_error_message as pesq_error_message
from .cypesq import PesqError, InvalidSampleRateError, OutOfMemoryError
from .cypesq import BufferTooShortError, NoUtterancesError

USAGE = """
        Run model on reference(ref) and degraded(deg)
        Sample rate (fs) - No default. Must select either 8000 or 16000.
        Note there is narrow band (nb) mode only when sampling rate is 8000Hz.
       """

USAGE_BATCH = USAGE + """
        The shapes of ref and deg should be identical if both are 2D numpy arrays.
        Once the `ref` is 1D  and the `deg` is 2D, the broadcast operation is applied. 
        """


def check_fs_mode(mode, fs, usage=USAGE):
    if mode != 'wb' and mode != 'nb':
        print(usage)
        raise ValueError("mode should be either 'nb' or 'wb'")

    if fs != 8000 and fs != 16000:
        print(usage)
        raise ValueError("fs (sampling frequency) should be either 8000 or 16000")

    if fs == 8000 and mode == 'wb':
        print(usage)
        raise ValueError("no wide band mode if fs = 8000")


def pesq_inner(fs, ref, deg, mode, on_error):
    """
    Args:
        ref: numpy 1D array, reference audio signal
        deg: numpy 1D array, degraded audio signal
        fs:  integer, sampling rate
        mode: 'wb' (wide-band) or 'nb' (narrow-band)
        on_error: PesqError.RAISE_EXCEPTION (default) or PesqError.RETURN_VALUES
    Returns:
        pesq_score: float, P.862.2 Prediction (MOS-LQO)
    """
    max_val = max(np.max(np.abs(ref / 1.0)), np.max(np.abs(deg / 1.0)))
    if mode == 'wb':
        mode_code = 1
    else:
        mode_code = 0
    if on_error == PesqError.RETURN_VALUES:
        return cypesq_retvals(
            fs,
            (ref / max_val).astype(np.float32),
            (deg / max_val).astype(np.float32),
            mode_code
        )
    return cypesq(
        fs,
        (ref / max_val).astype(np.float32),
        (deg / max_val).astype(np.float32),
        mode_code
    )


def pesq(fs, ref, deg, mode, on_error=PesqError.RAISE_EXCEPTION):
    """
    Args:
        ref: numpy 1D array, reference audio signal
        deg: numpy 1D array, degraded audio signal
        fs:  integer, sampling rate
        mode: 'wb' (wide-band) or 'nb' (narrow-band)
        on_error: PesqError.RAISE_EXCEPTION (default) or PesqError.RETURN_VALUES
    Returns:
        pesq_score: float, P.862.2 Prediction (MOS-LQO)
    """
    check_fs_mode(mode, fs, USAGE)
    return pesq_inner(fs, ref, deg, mode, on_error)


def pesq_batch(fs, ref, deg, mode, on_error=PesqError.RAISE_EXCEPTION):
    """
    Args:
        ref: numpy 1D (n_sample,) or 2D array (n_file, n_sample), reference audio signal
        deg: numpy 1D (n_sample,) or 2D array (n_file, n_sample), degraded audio signal
        fs:  integer, sampling rate
        mode: 'wb' (wide-band) or 'nb' (narrow-band)
        on_error: PesqError.RAISE_EXCEPTION (default) or PesqError.RETURN_VALUES
    Returns:
        pesq_score: numpy 1D array, P.862.2 Prediction (MOS-LQO)
    """
    check_fs_mode(mode, fs, USAGE_BATCH)
    # check dimension
    if len(ref.shape) == 1:
        if len(deg.shape) == 1 and ref.shape == deg.shape:
            return pesq_inner(fs, ref, deg, mode, on_error)
        elif len(deg.shape) == 2 and ref.shape[-1] == deg.shape[-1]:
            pesq_score = np.array([np.nan for i in range(deg.shape[0])])
            for i in range(deg.shape[0]):
                pesq_score[i] = pesq_inner(fs, ref, deg[i, :], mode, on_error)
            return pesq_score
        else:
            raise ValueError("The shapes of `deg` is invalid!")
    elif len(ref.shape) == 2:
        if deg.shape == ref.shape:
            pesq_score = np.array([np.nan for i in range(deg.shape[0])])
            for i in range(deg.shape[0]):
                pesq_score[i] = pesq_inner(fs, ref[i, :], deg[i, :], mode, on_error)
            return pesq_score
        else:
            raise ValueError("The shape of `deg` is invalid!")
    else:
        raise ValueError("The shape of `ref` should be either 1D or 2D!")
