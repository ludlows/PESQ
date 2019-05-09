# distutils: language=C

import cython
cimport numpy as np


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


cpdef object cypesq(long sample_rate, np.ndarray[float, ndim=1, mode="c"] ref_data,  np.ndarray[float, ndim=1, mode="c"] deg_data, int mode):
    # select rate
    cdef long error_flag = 0;
    cdef char * error_type = "unknown";
    select_rate(sample_rate, &error_flag, &error_type)
    if error_flag != 0:
        return -1
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
    if error_flag!=0:
        return -1
    return err_info.mapped_mos


