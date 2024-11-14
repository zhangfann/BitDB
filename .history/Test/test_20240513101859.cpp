#include <iostream>
#include <vector>
#include <random>
#include <ctime>

double get_random_price(double base, double range) {
    std::default_random_engine generator(std::time(0));
    std::uniform_real_distribution<double> distribution(-range, range);
    return base * (1 + distribution(generator));
}

double calculate_average(const std::vector<double>& prices, int days) {
    double sum = 0;
    for (int i = prices.size() - days; i < prices.size(); ++i) {
        sum += prices[i];
    }
    return sum / days;
}

double calculate_investment_ratio(double current_price, double avg_price) {
    if (current_price <= avg_price) {
        double ratio = (avg_price - current_price) / avg_price;
        if (ratio >= 0.4) return 1.6;
        if (ratio >= 0.3) return 1.5;
        if (ratio >= 0.2) return 1.4;
        if (ratio >= 0.1) return 1.3;
        if (ratio >= 0.05) return 1.2;
        return 1.1;
    } else {
        double ratio = (current_price - avg_price) / avg_price;
        if (ratio >= 0.1) return 0.6;
        if (ratio >= 0.5) return 0.7;
        if (ratio >= 0.9) return 0.8;
        if (ratio >= 1.5) return 0.9;
        if (ratio >= 2.5) return 1.0;
        return 1.0;
    }
}

int main() {
    const int initial_days = 300;
    const int total_days = 10000;
    const int strategy_start_day = initial_days;
    const double initial_price = 100.0;
    const double price_range = 0.1;
    const int moving_avg_days = 240;
    const double strategy_A_base_investment = 1000.0;
    const double strategy_B_investment = 1000.0;

    std::vector<double> prices(initial_days, initial_price);
    for (int i = initial_days; i < total_days; ++i) {
        prices[i] = get_random_price(prices[i - 1], price_range);
        if (i >= strategy_start_day) {
            // 策略A
            double avg_price = calculate_average(prices, moving_avg_days);
            double ratio = calculate_investment_ratio(prices[i], avg_price);
            double strategy_A_investment = strategy_A_base_investment * ratio;
            // 策略B
            double strategy_B_investment = strategy_B_investment;
            // 计算收益（这里省略了收益计算）
        }
    }

    // 输出最终收益（这里省略了收益计算和输出）
    return 0;
}