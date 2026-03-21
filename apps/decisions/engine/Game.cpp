

#include "Game.h"
#include "../directory_tools.h"
#include "csv.hpp"
#include <sstream>

#include "../../../external/zpp_bits/zpp_bits.hpp"

bool Game::LoadCashFlows(std::filesystem::path* path, TimeFormat fmt, const std::string& earlyDateName = "", std::string& lateDateName = "" )
{
    if (!directory_tools::dirExists(path))
        return false;

    csv::CSVReader reader(path->string());

    // Verify required columns are present
    auto col_names = reader.get_col_names();
    auto has_col = [&](const std::string& name) {
        return std::ranges::find(col_names, name) != col_names.end();
    };

    if (!has_col("amount") ||
        !has_col("time") ||
        !has_col("probability"))
        return false;


    for (csv::CSVRow& row : reader)
    {
        CashFlow<float, float> cf{};
        cf.amount      = row["amount"].get<float>();
        cf.probability = row["probability"].get<float>();

        // Parse time according to specified format
        if (fmt == TimeFormat::ISO8601)
        {
            std::tm tm{};
            std::istringstream ss(row["time"].get<std::string>());
            ss >> std::get_time(&tm, "%Y-%m-%d");
            tm.tm_isdst = -1; // let mktime determine DST
            cf.date = std::mktime(&tm);
        }
        else
        {
            cf.date = static_cast<time_t>(row["time"].get<long long>());
        }

        cashflows.push_back(cf);
    }

    return true;
}

bool Game::SaveGame( const Game& game , const std::filesystem::path* path)
{
    auto [data, out] = zpp::bits::data_out();
    auto result = out(game);

    if (zpp::bits::failure(result))
    {
        return false;
    }



    // Write to it
    std::ofstream saveFile(path->string(), std::ios::out | std::ios::binary);
    saveFile.write( reinterpret_cast<const std::ostream::char_type*>(&data[0]), data.size());
    saveFile.close();
    return true;
}

bool Game::LoadGame(Game& game, std::filesystem::path* path)
{


    if (directory_tools::dirExists(path))
    {
        std::ifstream readFile(path->string(), std::ios::in | std::ios::binary);
        std::vector<char> bufferContent ((std::istreambuf_iterator<char>(readFile)), std::istreambuf_iterator<char>());
        auto [in, out] = zpp::bits::in_out(bufferContent);


        in (game);


    }

  return true;

}

static void PrintTableHeader()
{
    std::cout << std::left
              << std::setw(14) << "Date"
              << std::setw(14) << "Amount"
              << std::setw(14) << "Probability"
              << "\n"
              << std::string(42, '-') << "\n";
}

static void PrintCashFlowRow(const CashFlow<float, float>& cf)
{
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&cf.date));
    std::cout << std::fixed << std::setprecision(2) << std::left
              << std::setw(14) << buf
              << std::setw(14) << cf.amount
              << std::setw(14) << cf.probability
              << "\n";
}

static void PrintCashFlowTable(const std::vector<CashFlow<float, float>>& rows, int start, int count, const std::string& label)
{
    std::cout << label << "\n";
    PrintTableHeader();
    for (int i = start; i < start + count; ++i)
        PrintCashFlowRow(rows[i]);
    std::cout << "\n";
}

void Game::PrintHead(int n) const
{
    int count = std::min(n, static_cast<int>(cashflows.size()));
    PrintCashFlowTable(cashflows, 0, count, "=== Head (" + std::to_string(count) + " of " + std::to_string(cashflows.size()) + ") ===");
}

void Game::PrintTail(int n) const
{
    int count = std::min(n, static_cast<int>(cashflows.size()));
    int start = static_cast<int>(cashflows.size()) - count;
    PrintCashFlowTable(cashflows, start, count, "=== Tail (" + std::to_string(count) + " of " + std::to_string(cashflows.size()) + ") ===");
}
