# distutils: language=C

#2019-May
#github.com/ludlows
#Python Wrapper for PESQ Score (narrow band and wide band)

import cython
cimport numpy as np 

class PesqError(RuntimeError):
    # Error Return Values
    SUCCESS                =  0
    UNKNOWN                = -1
    INVALID_SAMPLE_RATE    = -2
    OUT_OF_MEMORY_REF      = -3
    OUT_OF_MEMORY_DEG      = -4
    OUT_OF_MEMORY_TMP      = -5
    BUFFER_TOO_SHORT       = -6
    NO_UTTERANCES_DETECTED = -7

    # On Error Type
    RAISE_EXCEPTION = 0
    RETURN_VALUES   = 1

class InvalidSampleRateError(PesqError):
    pass

class OutOfMemoryError(PesqError):
    pass

class BufferTooShortError(PesqError):
    pass

class NoUtterancesError(PesqError):
    pass

cdef char** cypesq_error_messages = [
    "Success",
    "Unknown",
    "Invalid sampling rate",
    "Unable to allocate memory for reference buffer",
    "Unable to allocate memory for degraded buffer",
    "Unable to allocate memory for temporary buffer",
    "Buffer needs to be at least 1/4 of a second long",
    "No utterances detected"
]

cpdef char* cypesq_error_message(int code):
    global cypesq_error_messages
    
    if code > PesqError.SUCCESS:
        code = PesqError.SUCCESS

    if code < PesqError.NO_UTTERANCES_DETECTED:
        code = PesqError.UNKNOWN

    return cypesq_error_messages[-code]

cdef extern from "pesq.h":
    DEF MAXNUTTERANCES = 50
    DEF NB_MODE = 0
    DEF WB_MODE = 1

    ctypedef struct SIGNAL_INFO:
        char  path_name[512]
        char  file_name[128]
        long  Nsamples
        long  apply_swap
        long  input_filter

        float * data
        float * VAD
        float * logVAD
    
    ctypedef struct ERROR_INFO:
        long Nutterances
        long Largest_uttsize
        long Nsurf_samples

        long  Crude_DelayEst
        float Crude_DelayConf
        long  UttSearch_Start[MAXNUTTERANCES]
        long  UttSearch_End[MAXNUTTERANCES]
        long  Utt_DelayEst[MAXNUTTERANCES]
        long  Utt_Delay[MAXNUTTERANCES]
        float Utt_DelayConf[MAXNUTTERANCES]
        long  Utt_Start[MAXNUTTERANCES]
        long  Utt_End[MAXNUTTERANCES]

        float pesq_mos
        float mapped_mos
        short mode

cdef extern from "pesqio.h":
    cdef void select_rate(long sample_rate, long * Error_Flag, char ** Error_Type)


cdef extern from "pesqmain.h":
    cdef void pesq_measure(SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info, ERROR_INFO * err_info, long * Error_Flag, char ** Error_Type)



cpdef object cypesq_retvals(long sample_rate,
                    np.ndarray[float, ndim=1, mode="c"] ref_data,
                    np.ndarray[float, ndim=1, mode="c"] deg_data,
                    int mode):
    # select rate
    cdef long error_flag = 0;
    cdef char * error_type = "unknown";

    select_rate(sample_rate, &error_flag, &error_type)
    if error_flag != 0:
        # They are all literals, this is not a leak (probably)
        return PesqError.INVALID_SAMPLE_RATE

    # assign signal
    cdef long length_ref
    cdef long length_deg
    length_ref = ref_data.shape[0]
    length_deg = deg_data.shape[0]

    cdef SIGNAL_INFO ref_info, deg_info

    cdef char*  ref_name = "reference-signal"
    cdef char*  deg_name = "degrade-signal"
    
    cdef int i = 0;
    while ref_name[i]: 
        ref_info.path_name[i] = ref_name[i]
        ref_info.file_name[i] = ref_name[i]
        i += 1
    
    ref_info.path_name[i] = 0
    ref_info.file_name[i] = 0

    i = 0
    while deg_name[i]:
        deg_info.path_name[i] = deg_name[i]
        deg_info.file_name[i] = deg_name[i]
        i += 1
    
    deg_info.path_name[i] = 0
    deg_info.file_name[i] = 0


    ref_info.Nsamples = length_ref
    ref_info.apply_swap = 0
    ref_info.input_filter = 1
    ref_info.data = &(ref_data[0])


    deg_info.Nsamples = length_deg
    deg_info.apply_swap = 0
    deg_info.input_filter = 1
    deg_info.data = &(deg_data[0])

    # assign error info
    cdef ERROR_INFO err_info
    err_info.mode = NB_MODE
    if mode == 1:
        ref_info.input_filter = 2
        deg_info.input_filter = 2
        err_info.mode = WB_MODE

    pesq_measure(&ref_info, &deg_info, &err_info, &error_flag, &error_type);
    if error_flag != 0:
        return error_flag
    
    return err_info.mapped_mos

cpdef object cypesq(long sample_rate,
                    np.ndarray[float, ndim=1, mode="c"] ref_data,
                    np.ndarray[float, ndim=1, mode="c"] deg_data,
                    int mode):
    cdef object ret = cypesq_retvals(sample_rate, ref_data, deg_data, mode)

    # Null and Positive are valid values.
    if ret >= 0:
        return ret

    cdef char* error_message = cypesq_error_message(ret)

    if ret == PesqError.INVALID_SAMPLE_RATE:
        raise InvalidSampleRateError(error_message)
    
    if ret in [ PesqError.OUT_OF_MEMORY_REF, PesqError.OUT_OF_MEMORY_DEG, PesqError.OUT_OF_MEMORY_TMP ]:
        raise OutOfMemoryError(error_message)
    
    if ret == PesqError.BUFFER_TOO_SHORT:
        raise BufferTooShortError(error_message)
    
    if ret == PesqError.NO_UTTERANCES_DETECTED:
        raise NoUtterancesError(error_message)

    # Raise unknown otherwise
    raise PesqError(error_message)

