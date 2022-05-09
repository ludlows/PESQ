# 2019-May
# github.com/ludlows
# Python Wrapper for PESQ Score (narrowband and wideband)

import numpy as np
from multiprocessing import Pool, Queue, Process, cpu_count
from functools import partial
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


def _check_fs_mode(mode, fs, usage=USAGE):
    if mode != 'wb' and mode != 'nb':
        print(usage)
        raise ValueError("mode should be either 'nb' or 'wb'")

    if fs != 8000 and fs != 16000:
        print(usage)
        raise ValueError("fs (sampling frequency) should be either 8000 or 16000")

    if fs == 8000 and mode == 'wb':
        print(usage)
        raise ValueError("no wide band mode if fs = 8000")


def _pesq_inner(ref, deg, fs=16000, mode='wb', on_error=PesqError.RAISE_EXCEPTION):
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


def _processor_coordinator(func, args_q, results_q):
    while True:
        index, arg = args_q.get()
        if index is None:
            break
        try:
            result = func(*arg)
        except Exception as e:
            result = e
        results_q.put((index, result))


def _processor_mapping(func, args, n_processor):
    args_q = Queue(maxsize=1)
    results_q = Queue()
    processors = [Process(target=_processor_coordinator, args=(func, args_q, results_q)) for _ in range(n_processor)]
    for p in processors:
        p.daemon = True
        p.start()
    for i, arg in enumerate(args):
        args_q.put((i, arg))
    # send stop messages
    for _ in range(n_processor):
        args_q.put((None, None))
    results = [results_q.get() for _ in range(len(args))]
    [p.join() for p in processors]
    return [v[1] for v in sorted(results)]


def pesq(fs, ref, deg, mode='wb', on_error=PesqError.RAISE_EXCEPTION):
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
    _check_fs_mode(mode, fs, USAGE)
    return _pesq_inner(ref, deg, fs, mode, on_error)


def pesq_batch(fs, ref, deg, mode, n_processor=cpu_count(), on_error=PesqError.RAISE_EXCEPTION):
    """
    Running `pesq` using multiple processors
    Args:
        on_error:
        ref: numpy 1D (n_sample,) or 2D array (n_file, n_sample), reference audio signal
        deg: numpy 1D (n_sample,) or 2D array (n_file, n_sample), degraded audio signal
        fs:  integer, sampling rate
        mode: 'wb' (wide-band) or 'nb' (narrow-band)
        n_processor: cpu_count() (default) or number of processors (chosen by the user) or 0 (without multiprocessing)
        on_error: PesqError.RAISE_EXCEPTION (default) or PesqError.RETURN_VALUES
    Returns:
        pesq_score: list of pesq scores, P.862.2 Prediction (MOS-LQO)
    """
    _check_fs_mode(mode, fs, USAGE_BATCH)
    # check dimension
    if len(ref.shape) == 1:
        if len(deg.shape) == 1 and ref.shape == deg.shape:
            return [_pesq_inner(ref, deg, fs, mode, PesqError.RETURN_VALUES)]
        elif len(deg.shape) == 2 and ref.shape[-1] == deg.shape[-1]:
            if n_processor <= 0:
                pesq_score = [np.nan for i in range(deg.shape[0])]
                for i in range(deg.shape[0]):
                    pesq_score[i] = _pesq_inner(ref, deg[i, :], fs, mode, on_error)
                return pesq_score
            else:
                with Pool(n_processor) as p:
                    return p.map(partial(_pesq_inner, ref, fs=fs, mode=mode, on_error=on_error),
                                 [deg[i, :] for i in range(deg.shape[0])])
        else:
            raise ValueError("The shapes of `deg` is invalid!")
    elif len(ref.shape) == 2:
        if deg.shape == ref.shape:
            if n_processor <= 0:
                pesq_score = [np.nan for i in range(deg.shape[0])]
                for i in range(deg.shape[0]):
                    pesq_score[i] = _pesq_inner(ref[i, :], deg[i, :], fs, mode, on_error)
                return pesq_score
            else:
                return _processor_mapping(_pesq_inner,
                                          [(ref[i, :], deg[i, :], fs, mode, on_error) for i in range(deg.shape[0])],
                                          n_processor)
        else:
            raise ValueError("The shape of `deg` is invalid!")
    else:
        raise ValueError("The shape of `ref` should be either 1D or 2D!")
