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
        investment_ = investment_ + base_investment_ * ratio;
        shares_ += investment_ / price_;
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
// 每天获得一个价格
double get_random_price(double base, double range) {
    std::default_random_engine generator(std::time(0));
    std::uniform_real_distribution<double> distribution(-range, range);
    return base * (1 + distribution(generator));
}

// 前240天平均值
double calculate_average(const std::vector<double>& prices, int days) {
    double sum = 0;
    for (int i = prices.size() - days; i < prices.size(); ++i) {
        sum += prices[i];
    }
    return sum / days;
}

// 根据当前价格和均值计算投资比例
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
    const int initial_days = 1;
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
        std::cout << "Strategy " 
                  << ": Final value = " << final_value << std::endl;
    }

    return 0;
}