#include <algorithm>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

template <typename T>
class Connector;

template <typename T>
class Constraint : public std::enable_shared_from_this<Constraint<T>> {
public:
    void on_connector_value_change(std::shared_ptr<Connector<T>> connector,
                                   std::shared_ptr<Constraint<T>> change_maker) {
        if (change_maker == this->shared_from_this()) {
            return;
        }
        if (connector->has_value()) {
            on_connector_set_value(connector, connector->get_value());
        } else {
            on_connector_drop_value(connector);
        }
    }

protected:
    virtual void on_connector_set_value(std::shared_ptr<Connector<T>> connector, const T &value){};
    virtual void on_connector_drop_value(std::shared_ptr<Connector<T>> connector){};
};

template <typename T>
class Connector : public std::enable_shared_from_this<Connector<T>> {
public:
    virtual ~Connector() = default;

    bool has_value() const {
        return _value.has_value();
    }

    const T &get_value() const {
        return _value.value();
    }

    void set_value(const T &value, std::shared_ptr<Constraint<T>> change_maker = nullptr) {
        _value = value;
        notify_value_change(change_maker);
    }

    void drop_value(std::shared_ptr<Constraint<T>> change_maker = nullptr) {
        _value = std::nullopt;
        notify_value_change(change_maker);
    }

    void connect(std::shared_ptr<Constraint<T>> constraint) {
        _constraints.insert(constraint);
    }

private:
    std::optional<T> _value;
    std::optional<T> _computed_value; // TODO
    std::unordered_set<std::shared_ptr<Constraint<T>>> _constraints;

    void notify_value_change(std::shared_ptr<Constraint<T>> change_maker) {
        for (auto &constraint : _constraints) {
            constraint->on_connector_value_change(this->shared_from_this(), change_maker);
        }
    }
};

template <typename T>
class Adder : public Constraint<T> {
public:
    void set_lhs(std::shared_ptr<Connector<T>> lhs) {
        _lhs = lhs;
        lhs->connect(this->shared_from_this());
    }

    void set_rhs(std::shared_ptr<Connector<T>> rhs) {
        _rhs = rhs;
        rhs->connect(this->shared_from_this());
    }

    void set_sum(std::shared_ptr<Connector<T>> sum) {
        _sum = sum;
        sum->connect(this->shared_from_this());
    }

protected:
    void on_connector_set_value(std::shared_ptr<Connector<T>> connector, const T &value) override {
        if (_lhs->has_value() && _rhs->has_value() && !_sum->has_value()) {
            _sum->set_value(_lhs->get_value() + _rhs->get_value(), this->shared_from_this());
        } else if (_lhs->has_value() && !_rhs->has_value() && _sum->has_value()) {
            _rhs->set_value(_sum->get_value() - _lhs->get_value(), this->shared_from_this());
        } else if (!_lhs->has_value() && _rhs->has_value() && _sum->has_value()) {
            _lhs->set_value(_sum->get_value() - _rhs->get_value(), this->shared_from_this());
        } else if (_lhs->has_value() && _rhs->has_value() && _sum->has_value()) {
            assert(_lhs->get_value() + _rhs->get_value() == _sum->get_value());
        }
    }

private:
    std::shared_ptr<Connector<T>> _lhs;
    std::shared_ptr<Connector<T>> _rhs;
    std::shared_ptr<Connector<T>> _sum;
};

int main(int argc, const char *argv[]) {
    auto a = std::make_shared<Connector<int>>();
    auto b = std::make_shared<Connector<int>>();
    auto c = std::make_shared<Connector<int>>();

    auto adder = std::make_shared<Adder<int>>();
    adder->set_lhs(a);
    adder->set_rhs(b);
    adder->set_sum(c);

    a->set_value(1);
    b->set_value(2);
    std::cout << "c = " << c->get_value() << std::endl;

    c->drop_value();
    a->drop_value();
    c->set_value(10);
    std::cout << "a = " << a->get_value() << std::endl;

    return 0;
}
