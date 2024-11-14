#include <iostream>
#include <vector>
#include <random>
#include <ctime>

// 定义策略类
class Strategy {
public:
    Strategy(double base_investment, double investment, double price)
        : base_investment_(base_investment), investment_(investment), price_(price), shares_(0) {}

    void invest(double ratio) {
        investment_ *= ratio;
        shares_ = investment_ / price_;
    }

    double total_investment() const { return investment_; }
    double final_shares() const { return shares_; }
    double final_price() const { return price_; }

private:
    double base_investment_;
    double investment_;
    double shares_;
    double price_;
};

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
    std::vector<Strategy> strategies = { Strategy(strategy_A_base_investment, 0, initial_price),
                                         Strategy(strategy_B_investment, 0, initial_price) };

    for (int i = initial_days; i < total_days; ++i) {
        prices[i] = get_random_price(prices[i - 1], price_range);

        if (i >= strategy_start_day) {
            // 策略A
            double avg_price = calculate_average(prices, moving_avg_days);
            double ratio = calculate_investment_ratio(prices[i], avg_price);
            strategies[0].invest(ratio);

            // 策略B
            strategies[1].invest(1.0);  // 固定投资
        }
    }

    // 输出每个策略的收益
    for (const auto& strategy : strategies) {
        double final_value = strategy.final_shares() * prices.back() - strategy.total_investment();
        std::cout << "Strategy " << (strategy == strategies[0] ? "A" : "B")
                  << ": Final value = " << final_value << std::endl;
    }

    return 0;
}