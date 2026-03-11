#pragma once
#include <ctime>
#include <filesystem>
#include <vector>
#include "../data_types/cashflow.h"
#include <iostream>
#include <iomanip>

#include "../../../external/zpp_bits/zpp_bits.hpp"

enum class TimeFormat
{
    UnixTimestamp, // e.g. 1773532800
    ISO8601,       // e.g. 2026-03-15
};

class Game
{

    public:
        Game(const time_t _startTime, const time_t _endTime): startTime(_startTime), endTime(_endTime) {}
        time_t startTime;
        time_t endTime;
        double accountBalance = 0;
        std::vector<CashFlow<float, float>> cashflows = {};

        using serialize = zpp::bits::members<4>;

        bool LoadCashFlows(std::filesystem::path* path, TimeFormat fmt = TimeFormat::ISO8601);
        static bool SaveGame(const Game& game, const std::filesystem::path* path);
        static bool LoadGame(Game& game, std::filesystem::path* path);

        void PrintHead(int n = 5) const;
        void PrintTail(int n = 5) const;

};
