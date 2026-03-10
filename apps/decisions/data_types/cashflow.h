#pragma once
#include <ctime>


template <typename c_type, typename p_type> struct CashFlow
{

public:
    // Amount
    c_type amount;
    // Date
    time_t time;
    // Probability of occurring
    p_type probability;
} ;
