#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
using namespace std;

class Expression;
using ExprPtr = shared_ptr<Expression>;

class Expression {
public:
    virtual ~Expression() = default;

    virtual double evaluate(double x) const = 0;
    virtual ExprPtr derivative() const = 0;
    virtual string toString() const = 0;

    virtual bool isConstant() const {
        return false;
    }
    virtual double constantValue() const {
        return 0.0;
    }
    bool isZero() const {
        return isConstant() && constantValue() == 0.0;
    }
    bool isOne() const {
        return isConstant() && constantValue() == 1.0;
    }
};

ExprPtr constant(double v);
ExprPtr add(ExprPtr a, ExprPtr b);
ExprPtr sub(ExprPtr a, ExprPtr b);
ExprPtr mul(ExprPtr a, ExprPtr b);
ExprPtr divide(ExprPtr a, ExprPtr b);
ExprPtr power(ExprPtr a, ExprPtr b);

class Constant : public Expression {
    double value_;
public:
    Constant(double value) : value_(value) {}

    double evaluate(double) const override {
        return value_;
    }
    ExprPtr derivative() const override {
        return make_shared<Constant>(0);
    }
    string toString() const override {
        ostringstream os;
        os << value_;
        return os.str();
    }
    bool isConstant() const override {
        return true;
    }
    double constantValue() const override {
        return value_;
    }
};

class Variable : public Expression {
    string name_;
public:
    Variable(string name = "x") : name_(move(name)) {}

    double evaluate(double x) const override {
        return x;
    }
    ExprPtr derivative() const override {
        return make_shared<Constant>(1);
    }
    string toString() const override {
        return name_;
    }
};

class BinaryOperation : public Expression {
protected:
    ExprPtr left_, right_;
    virtual string symbol() const = 0;
public:
    BinaryOperation(ExprPtr l, ExprPtr r) : left_(move(l)), right_(move(r)) {}

    string toString() const override {
        return "(" + left_->toString() + " " + symbol() + " " + right_->toString() + ")";
    }
};

class Add : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;

    double evaluate(double x) const override {
        return left_->evaluate(x) + right_->evaluate(x);
    }

    ExprPtr derivative() const override {
        return add(left_->derivative(), right_->derivative());
    }
protected:
    string symbol() const override { return "+"; }
};

class Subtract : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;

    double evaluate(double x) const override {
        return left_->evaluate(x) - right_->evaluate(x);
    }

    ExprPtr derivative() const override {
        return sub(left_->derivative(), right_->derivative());
    }
protected:
    string symbol() const override { return "-"; }
};

class Multiply : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;

    double evaluate(double x) const override {
        return left_->evaluate(x) * right_->evaluate(x);
    }

    ExprPtr derivative() const override {
        return add(mul(left_->derivative(), right_),
                   mul(left_, right_->derivative()));
    }
protected:
    string symbol() const override {
        return "*";
    }
};

class Divide : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;

    double evaluate(double x) const override {
        return left_->evaluate(x) / right_->evaluate(x);
    }
    ExprPtr derivative() const override {
        return divide(sub(mul(left_->derivative(), right_),
                          mul(left_, right_->derivative())),
                      power(right_, constant(2)));
    }
protected:
    string symbol() const override {
        return "/";
    }
};

class Power : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;

    double evaluate(double x) const override {
        return pow(left_->evaluate(x), right_->evaluate(x));
    }
    ExprPtr derivative() const override;
protected:
    string symbol() const override {
        return "^";
    }
};

class UnaryFunction : public Expression {
protected:
    ExprPtr arg_;
    virtual string name() const = 0;
    virtual ExprPtr outerDerivative() const = 0;
public:
    explicit UnaryFunction(ExprPtr arg) : arg_(move(arg)) {}

    // Regla de la cadena:
    ExprPtr derivative() const override {
        return mul(outerDerivative(), arg_->derivative());
    }
    string toString() const override {
        return name() + "(" + arg_->toString() + ")";
    }
};

class Sin;
class Cos;

class Cos : public UnaryFunction {
public:
    using UnaryFunction::UnaryFunction;
    double evaluate(double x) const override {
        return cos(arg_->evaluate(x));
    }
protected:
    string name() const override {
        return "cos";
    }
    ExprPtr outerDerivative() const override;
};

class Sin : public UnaryFunction {
public:
    using UnaryFunction::UnaryFunction;
    double evaluate(double x) const override {
        return sin(arg_->evaluate(x));
    }
protected:
    string name() const override {
        return "sin";
    }
    ExprPtr outerDerivative() const override {
        return make_shared<Cos>(arg_);
    }
};

ExprPtr Cos::outerDerivative() const {
    return mul(constant(-1), make_shared<Sin>(arg_));
}

class Exp : public UnaryFunction {
public:
    using UnaryFunction::UnaryFunction;
    double evaluate(double x) const override {
        return exp(arg_->evaluate(x));
    }
protected:
    string name() const override {
        return "exp";
    }
    ExprPtr outerDerivative() const override {
        return make_shared<Exp>(arg_);
    }
};

class Ln : public UnaryFunction {
public:
    using UnaryFunction::UnaryFunction;
    double evaluate(double x) const override {
        return log(arg_->evaluate(x));
    }
protected:
    string name() const override {
        return "ln";
    }
    ExprPtr outerDerivative() const override {
        return divide(constant(1), arg_);
    }
};

//Potencia
ExprPtr Power::derivative() const {
    if (right_->isConstant()) {
        double n = right_->constantValue();
        return mul(mul(constant(n), power(left_, constant(n - 1))),
                   left_->derivative());
    }
    // General: (u^v)' = u^v * (v' ln(u) + v u'/u)
    ExprPtr self = make_shared<Power>(left_, right_);
    ExprPtr term = add(mul(right_->derivative(), make_shared<Ln>(left_)),
                       divide(mul(right_, left_->derivative()), left_));
    return mul(self, term);
}

ExprPtr constant(double v) {
    return make_shared<Constant>(v);
}

ExprPtr add(ExprPtr a, ExprPtr b) {
    if (a->isZero()) return b;
    if (b->isZero()) return a;
    return make_shared<Add>(a, b);
}

ExprPtr sub(ExprPtr a, ExprPtr b) {
    if (b->isZero()) return a;
    return make_shared<Subtract>(a, b);
}

ExprPtr mul(ExprPtr a, ExprPtr b) {
    if (a->isZero() || b->isZero()) return constant(0);
    if (a->isOne()) return b;
    if (b->isOne()) return a;
    return make_shared<Multiply>(a, b);
}

ExprPtr divide(ExprPtr a, ExprPtr b) {
    if (a->isZero()) return constant(0);
    if (b->isOne()) return a;
    return make_shared<Divide>(a, b);
}

ExprPtr power(ExprPtr a, ExprPtr b) {
    if (b->isZero()) return constant(1);
    if (b->isOne()) return a;
    return make_shared<Power>(a, b);
}


int main() {
    auto x = std::make_shared<Variable>();
    auto expr =
    std::make_shared<Multiply>(
    std::make_shared<Add>(
    std::make_shared<Power>(
    x,
    std::make_shared<Constant>(2)
    ),
    std::make_shared<Multiply>(
    std::make_shared<Constant>(3),
    x
    )
    ),
    std::make_shared<Sin>(x)
    );
    auto deriv = expr->derivative();
    std::cout << "f(x) = " << expr->toString() << std::endl;
    std::cout << "f'(x) = " << deriv->toString() << std::endl;
    std::cout << "f(2) = " << expr->evaluate(2) << std::endl;
}