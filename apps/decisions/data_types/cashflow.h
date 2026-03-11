#pragma once
#include <ctime>

#include "../../../external/zpp_bits/zpp_bits.hpp"


template <typename c_type, typename p_type> struct CashFlow
{

public:
    // Amount
    c_type amount;
    // Date
    time_t time;
    // Probability of occurring
    p_type probability;


    using serialize = zpp::bits::members<3>;



} ;
