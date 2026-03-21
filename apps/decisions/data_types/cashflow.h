#pragma once
#include <ctime>

#include "../../../external/zpp_bits/zpp_bits.hpp"


template <typename c_type, typename p_type> struct CashFlow
{

public:
    // Amount
    c_type amount;


    // Date
    time_t date;

    //Alternative early_date and late_date can be used for ranged transactions
    time_t early_date;
    time_t late_date;

    // Probability of occurring
    p_type probability;


    using serialize = zpp::bits::members<3>;



} ;
